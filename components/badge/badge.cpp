/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2023-2024 Jérémie Galarneau <jeremie.galarneau@gmail.com>
 */

#include "badge.hpp"
#include "badge-network/network_messages.hpp"
#include <badge/globals.hpp>

#include <algorithm>
#include <cstring>

namespace nr = nsec::runtime;
namespace nc = nsec::communication;
namespace nb = nsec::button;

namespace
{
constexpr uint16_t config_version_magic = 0xBAD8;
} // anonymous namespace

nr::badge::badge()
    : _is_user_name_set{false}, _user_name{""},
      _button_watcher([](nsec::button::id id, nsec::button::event event) {
          nsec::g::the_badge.on_button_event(id, event);
      }),
      _network_handler()
{
    _network_app_state(network_app_state::UNCONNECTED);
    _id_exchanger.reset();
    set_social_level(nsec::config::social::initial_level, false);
    _set_selected_animation(0, false);
}

void nr::badge::load_config()
{
    // FIXME Restore config
    eeprom_config config{};

    set_social_level(config.social_level, false);
    if (config.is_name_set) {
        std::memcpy(_user_name, config.name, sizeof(_user_name));
        _is_user_name_set = config.is_name_set;
    }

    _set_selected_animation(config.favorite_animation_id, false);
}

void nr::badge::save_config() const
{
    eeprom_config config;
    config.version_magic = config_version_magic;
    config.favorite_animation_id = _selected_animation;
    config.is_name_set = _is_user_name_set;
    config.social_level = _social_level;

    std::memcpy(config.name, _user_name, sizeof(config.name));
    // FIXME Save config
}

void nr::badge::factory_reset()
{
    // FIXME Clear stored config
    // Set an invalid magic in the config to cause validation to fail

    // FIXME There's surely a better way to do this on FreeRTOS
    void (*so_looooong)(void) = nullptr;
    so_looooong();
}

void nr::badge::setup()
{
    _button_watcher.setup();

    load_config();
}

uint8_t nr::badge::level() const noexcept
{
    return _social_level;
}

bool nr::badge::is_connected() const noexcept
{
    return _network_app_state() != network_app_state::UNCONNECTED;
}

void nr::badge::on_button_event(nsec::button::id button,
                                nsec::button::event event) noexcept
{
    const auto button_mask_position = static_cast<unsigned int>(button);

    if (_network_app_state() != network_app_state::UNCONNECTED &&
        _network_app_state() != network_app_state::IDLE) {
        // Don't allow button press during "modal" states.
        return;
    }

    /*
     * After a focus change, don't spam the newly focused screen with
     * repeat events of the button. We want to let the user the time to react,
     * take their meaty appendage off the button, and initiate new interactions.
     */
    if (event != nsec::button::event::DOWN_REPEAT) {
        _button_had_non_repeat_event_since_screen_focus_change |=
            1 << button_mask_position;
    }

    const auto filter_out_button_event =
        event == nsec::button::event::DOWN_REPEAT &&
        !(_button_had_non_repeat_event_since_screen_focus_change &
          (1 << button_mask_position));

    // Forward the event to the focused screen.
    if (!filter_out_button_event) {
        // FIXME Forward button to focused mode
    }
}

void nr::badge::set_social_level(uint8_t new_level, bool save) noexcept
{
    new_level =
        std::clamp(new_level, uint8_t(0), nsec::config::social::max_level);

    _social_level = new_level;
    if (save) {
        save_config();
    }
}

void nr::badge::on_disconnection() noexcept
{
    _network_app_state(network_app_state::UNCONNECTED);
}

void nr::badge::on_pairing_begin() noexcept
{
}

void nr::badge::on_pairing_end(nc::peer_id_t our_peer_id,
                               uint8_t peer_count) noexcept
{
    _network_app_state(network_app_state::ANIMATE_PAIRING);
}

nc::network_handler::application_message_action
nr::badge::on_message_received(communication::message::type message_type,
                               const uint8_t *message) noexcept
{
    if (_network_app_state() == network_app_state::EXCHANGING_IDS) {
        _id_exchanger.new_message(*this, message_type, message);
    } else if (_network_app_state() == network_app_state::ANIMATE_PAIRING) {
        _pairing_animator.new_message(*this, message_type, message);
    }

    return nc::network_handler::application_message_action::OK;
}

void nr::badge::on_app_message_sent() noexcept
{
    if (_network_app_state() == network_app_state::EXCHANGING_IDS) {
        _id_exchanger.message_sent(*this);
    }
}

void nr::badge::on_splash_complete() noexcept
{
}

nr::badge::network_app_state nr::badge::_network_app_state() const noexcept
{
    return network_app_state(_current_network_app_state);
}

void nr::badge::_network_app_state(
    nr::badge::network_app_state new_state) noexcept
{
    _id_exchanger.reset();
    _pairing_animator.reset();
    _pairing_completed_animator.reset();

    _current_network_app_state = uint8_t(new_state);
    switch (new_state) {
    case network_app_state::ANIMATE_PAIRING:
    case network_app_state::EXCHANGING_IDS:
        if (new_state == network_app_state::ANIMATE_PAIRING) {
            _pairing_animator.start(*this);
        } else {
            _id_exchanger.start(*this);
        }

        break;
    case network_app_state::ANIMATE_PAIRING_COMPLETED:
        _pairing_completed_animator.start(*this);
        break;
    case network_app_state::IDLE:
    case network_app_state::UNCONNECTED:
        break;
    }
}

nr::badge::badge_discovered_result
nr::badge::on_badge_discovered(const uint8_t *id) noexcept
{
    // FIXME How do we want to derive a unique badge ID?
    return badge_discovered_result::NEW;
}

void nr::badge::on_badge_discovery_completed() noexcept
{
    _badges_discovered_last_exchange = _id_exchanger.new_badges_discovered();
    _network_app_state(network_app_state::ANIMATE_PAIRING_COMPLETED);
}

void nr::badge::network_id_exchanger::start(nr::badge &badge) noexcept
{
    const auto our_id = badge._network_handler.peer_id();

    if (our_id != 0) {
        // Wait for messages from left neighbors.
        return;
    }

    // Left-most peer initiates the exchange.
    nc::message::announce_badge_id msg = {.peer_id = our_id};

    badge._network_handler.enqueue_app_message(
        nc::peer_relative_position::RIGHT,
        uint8_t(nc::message::type::ANNOUNCE_BADGE_ID),
        reinterpret_cast<const uint8_t *>(&msg));
}

void nr::badge::network_id_exchanger::new_message(
    nr::badge &badge, nc::message::type msg_type,
    const uint8_t *payload) noexcept
{
    if (msg_type != nc::message::type::ANNOUNCE_BADGE_ID) {
        return;
    }

    const auto *announce_badge_id =
        reinterpret_cast<const nc::message::announce_badge_id *>(payload);

    if (badge.on_badge_discovered(announce_badge_id->board_unique_id) ==
        badge_discovered_result::NEW) {
        _new_badges_discovered++;
    }

    _message_received_count++;

    const auto our_position = badge._network_handler.position();
    const auto our_peer_id = badge._network_handler.peer_id();
    const auto msg_origin_peer_id = announce_badge_id->peer_id;
    const auto peer_count = badge._network_handler.peer_count();

    switch (our_position) {
    case nc::network_handler::link_position::LEFT_MOST:
        if (_message_received_count == peer_count - 1) {
            // Done!
            badge.on_badge_discovery_completed();
        }

        break;
    case nc::network_handler::link_position::RIGHT_MOST:
        if (_message_received_count == peer_count - 1) {
            nc::message::announce_badge_id msg = {
                .peer_id = badge._network_handler.peer_id()};
            // FIXME Set unique ID.

            badge._network_handler.enqueue_app_message(
                nc::peer_relative_position(_direction),
                uint8_t(nc::message::type::ANNOUNCE_BADGE_ID),
                reinterpret_cast<const uint8_t *>(&msg));

            badge.on_badge_discovery_completed();
        }

        break;
    case nc::network_handler::link_position::MIDDLE: {
        if (abs(our_peer_id - msg_origin_peer_id) == 1) {
            _send_ours_on_next_send_complete = true;
            _done_after_sending_ours = msg_origin_peer_id > our_peer_id;
            _direction = msg_origin_peer_id < our_peer_id
                             ? uint8_t(nc::peer_relative_position::RIGHT)
                             : uint8_t(nc::peer_relative_position::LEFT);
        }

        // Forward messages from other badges.
        badge._network_handler.enqueue_app_message(
            msg_origin_peer_id < our_peer_id ? nc::peer_relative_position::RIGHT
                                             : nc::peer_relative_position::LEFT,
            uint8_t(msg_type), payload);
    }
    default:
        // Unreachable.
        return;
    }
}

void nr::badge::network_id_exchanger::message_sent(nr::badge &badge) noexcept
{
    if (!_send_ours_on_next_send_complete) {
        return;
    }

    nc::message::announce_badge_id msg = {.peer_id =
                                              badge._network_handler.peer_id()};
    // FIXME Set unique ID.

    badge._network_handler.enqueue_app_message(
        nc::peer_relative_position(_direction),
        uint8_t(nc::message::type::ANNOUNCE_BADGE_ID),
        reinterpret_cast<const uint8_t *>(&msg));

    _send_ours_on_next_send_complete = false;

    if (_done_after_sending_ours) {
        badge.on_badge_discovery_completed();
    }
}

void nr::badge::network_id_exchanger::reset() noexcept
{
    _new_badges_discovered = 0;
    _message_received_count = 0;
    _send_ours_on_next_send_complete = false;
    _done_after_sending_ours = false;
}

nr::badge::pairing_animator::pairing_animator()
{
    _animation_state(animation_state::DONE);
}

void nr::badge::pairing_animator::_animation_state(
    animation_state new_state) noexcept
{
    _current_state = uint8_t(new_state);
    _state_counter = 0;
}

nr::badge::pairing_animator::animation_state
nr::badge::pairing_animator::_animation_state() const noexcept
{
    return animation_state(_current_state);
}

void nr::badge::pairing_animator::start(nr::badge &badge) noexcept
{
    if (badge._network_handler.position() ==
        nc::network_handler::link_position::LEFT_MOST) {
        _animation_state(animation_state::LIGHT_UP_UPPER_BAR);
    } else {
        _animation_state(animation_state::WAIT_MESSAGE_ANIMATION_PART_1);
    }

    badge._timer.period_ms(
        nsec::config::badge::pairing_animation_time_per_led_progress_bar_ms);
}

void nr::badge::pairing_animator::reset() noexcept
{
    _animation_state(animation_state::DONE);
}

nr::badge::animation_task::animation_task() : periodic_task(250)
{
    periodic_task::start();
}

void nr::badge::animation_task::tick(
    nsec::scheduling::absolute_time_ms current_time_ms) noexcept
{
    nsec::g::the_badge.tick(current_time_ms);
}

void nr::badge::pairing_animator::tick(
    nsec::scheduling::absolute_time_ms current_time_ms) noexcept
{
    switch (_animation_state()) {
    case animation_state::DONE:
        if (_state_counter < 8 &&
            nsec::g::the_badge._network_handler.position() ==
                nc::network_handler::link_position::LEFT_MOST) {
            // Left-most badge waits for the neighbor to transition states.
            _state_counter++;
        } else {
            nsec::g::the_badge._network_app_state(
                nr::badge::network_app_state::EXCHANGING_IDS);
        }
        break;
    case animation_state::WAIT_MESSAGE_ANIMATION_PART_1:
    case animation_state::WAIT_MESSAGE_ANIMATION_PART_2:
    case animation_state::WAIT_DONE:
        break;
    case animation_state::LIGHT_UP_UPPER_BAR:
        if (_state_counter < 8) {
            _state_counter++;
        } else if (nsec::g::the_badge._network_handler.position() ==
                   nc::network_handler::link_position::RIGHT_MOST) {
            _animation_state(animation_state::LIGHT_UP_LOWER_BAR);
        } else {
            nsec::g::the_badge._network_handler.enqueue_app_message(
                nc::peer_relative_position::RIGHT,
                uint8_t(nc::message::type::PAIRING_ANIMATION_PART_1_DONE),
                nullptr);
            _animation_state(animation_state::WAIT_MESSAGE_ANIMATION_PART_2);
        }

        break;
    case animation_state::LIGHT_UP_LOWER_BAR:
        if (_state_counter < 8) {
            _state_counter++;
        } else if (nsec::g::the_badge._network_handler.position() ==
                   nc::network_handler::link_position::LEFT_MOST) {
            nsec::g::the_badge._network_handler.enqueue_app_message(
                nc::peer_relative_position::RIGHT,
                uint8_t(nc::message::type::PAIRING_ANIMATION_DONE), nullptr);

            _animation_state(animation_state::DONE);
        } else {
            nsec::g::the_badge._network_handler.enqueue_app_message(
                nc::peer_relative_position::LEFT,
                uint8_t(nc::message::type::PAIRING_ANIMATION_PART_2_DONE),
                nullptr);
            _animation_state(animation_state::WAIT_DONE);
        }

        break;
    }
}

void nr::badge::pairing_animator::new_message(nr::badge &badge,
                                              nc::message::type msg_type,
                                              const uint8_t *payload) noexcept
{
    switch (msg_type) {
    case nc::message::type::PAIRING_ANIMATION_PART_1_DONE:
        _animation_state(animation_state::LIGHT_UP_UPPER_BAR);
        break;
    case nc::message::type::PAIRING_ANIMATION_PART_2_DONE:
        _animation_state(animation_state::LIGHT_UP_LOWER_BAR);
        break;
    case nc::message::type::PAIRING_ANIMATION_DONE:
        if (nsec::g::the_badge._network_handler.position() !=
            nc::network_handler::link_position::RIGHT_MOST) {
            nsec::g::the_badge._network_handler.enqueue_app_message(
                nc::peer_relative_position::RIGHT,
                uint8_t(nc::message::type::PAIRING_ANIMATION_DONE), nullptr);
        }

        _animation_state(animation_state::DONE);
        break;
    default:
        break;
    }
}

void nr::badge::tick(
    nsec::scheduling::absolute_time_ms current_time_ms) noexcept
{
    switch (_network_app_state()) {
    case network_app_state::ANIMATE_PAIRING:
        _pairing_animator.tick(current_time_ms);
        break;
    case network_app_state::ANIMATE_PAIRING_COMPLETED:
        _pairing_completed_animator.tick(*this, current_time_ms);
        break;
    default:
        break;
    }
}

void nr::badge::pairing_completed_animator::start(nr::badge &badge) noexcept
{
    badge._timer.period_ms(1000);
}

void nr::badge::pairing_completed_animator::reset() noexcept
{
    _animation_state(animation_state::SHOW_PAIRING_COMPLETE_MESSAGE);
    _state_counter = 0;
}

void nr::badge::pairing_completed_animator::tick(
    nr::badge &badge,
    nsec::scheduling::absolute_time_ms current_time_ms) noexcept
{
    _state_counter++;
    if (_state_counter == 8) {
        _state_counter = 0;
        if (_animation_state() ==
            animation_state::SHOW_PAIRING_COMPLETE_MESSAGE) {
            _animation_state(animation_state::SHOW_NEW_LEVEL);
            memset(current_message, 0, sizeof(current_message));
        } else {
            badge.apply_score_change(badge._badges_discovered_last_exchange);
            badge._network_app_state(network_app_state::IDLE);
        }
    }
}

void nr::badge::pairing_completed_animator::_animation_state(
    nr::badge::pairing_completed_animator::animation_state new_state) noexcept
{
    _current_state = uint8_t(new_state);
}

nr::badge::pairing_completed_animator::animation_state
nr::badge::pairing_completed_animator::_animation_state() const noexcept
{
    return animation_state(_current_state);
}

void nr::badge::apply_score_change(uint8_t new_badges_discovered_count) noexcept
{
    // Saves to configuration
    set_social_level(
        _compute_new_social_level(_social_level, new_badges_discovered_count),
        true);
    _set_selected_animation(_social_level, true);
}

void nr::badge::show_badge_info() noexcept
{
}

uint8_t nr::badge::_compute_new_social_level(
    uint8_t current_social_level, uint8_t new_badges_discovered_count) noexcept
{
    uint16_t new_social_level = current_social_level;
    uint16_t level_up = new_badges_discovered_count;

    if (new_badges_discovered_count > 1) {
        level_up = new_badges_discovered_count *
                   nsec::config::social::
                       multiple_badges_discovered_simultaneously_multiplier;
    }

    new_social_level += level_up;

    return std::clamp<std::uint8_t>(new_social_level, 0,
                                    nsec::config::social::max_level);
}

void nr::badge::_set_selected_animation(uint8_t animation_id,
                                        bool save_to_config) noexcept
{
    _selected_animation = animation_id;

    if (save_to_config) {
        save_config();
    }
}

void nr::badge::cycle_selected_animation(
    nr::badge::cycle_animation_direction direction) noexcept
{
    const auto selected_animation = std::clamp(
        _selected_animation + int8_t(direction), 0, _social_level - 1);

    _set_selected_animation(selected_animation, true);
}
