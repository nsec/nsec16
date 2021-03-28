#include "rpg/characters/CharacterSoldier.h"

#include "rpg/Viewport.h"

#include "graphics.h"

namespace rpg
{

void CharacterSoldier::post_render(Viewport &viewport)
{
    if (current_mode == Mode::walking)
        moving_mixin();

    Character::post_render(viewport);
}

void CharacterSoldier::render(Viewport &viewport)
{
    if (!is_visible(viewport))
        return;

    if (current_mode == Mode::standing) {
        render_standing(viewport);
    } else {
        render_walking(viewport);
    }

    Character::render(viewport);
}

void CharacterSoldier::render_standing(Viewport &viewport)
{
    auto screen = viewport.to_screen(get_coordinates());
    graphics_draw_from_library(LIBRARY_IMAGE_SOLDIER_STANDING, screen.x(),
                               screen.y());

    if (timer.has_expired(get_animation_step())) {
        switch (get_animation_step() % 2) {
        case 0:
            current_mode = Mode::walking;
            timer.start(get_animation_step(), 11, 2);

            switch (get_animation_step() % 8) {
            case 0:
            case 1:
            case 2:
                set_moving_direction(direction::left);
                break;

            case 3:
            case 4:
            case 5:
                set_moving_direction(direction::right);
                break;

            case 6:
                set_moving_direction(direction::up);
                break;

            case 7:
            default:
                set_moving_direction(direction::down);
                break;
            }

            break;

        default:
            current_mode = Mode::standing;
            timer.start(get_animation_step(), 18, 6);
        }
    }
}

void CharacterSoldier::render_walking(Viewport &viewport)
{
    switch (get_moving_direction()) {
    case direction::left:
        render_animation_variant(viewport, Appearance::moving_left, 4);
        break;

    case direction::right:
        render_animation_variant(viewport, Appearance::moving_right, 4);
        break;

    case direction::up:
        render_animation_variant(viewport, Appearance::moving_up, 5);
        break;

    case direction::down:
    default:
        render_animation_variant(viewport, Appearance::moving_down, 5);
    }

    if (timer.has_expired(get_animation_step())) {
        current_mode = Mode::standing;
        timer.start(get_animation_step(), 10, 3);
    }
}

} // namespace rpg
