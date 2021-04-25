#pragma once

#include "rpg/Character.h"
#include "rpg/characters/MovingMixin.h"

#include "graphics.h"

namespace rpg::animation
{

static constexpr int aristocrate_moving_down[] = {
    LIBRARY_IMAGE_ARISTO_DOWN_1,
    LIBRARY_IMAGE_ARISTO_DOWN_2,
};

static constexpr int aristocrate_moving_left[] = {
    LIBRARY_IMAGE_ARISTO_LEFT_1,
    LIBRARY_IMAGE_ARISTO_LEFT_2,
    LIBRARY_IMAGE_ARISTO_LEFT_1,
    LIBRARY_IMAGE_ARISTO_LEFT_3,
};

static constexpr int aristocrate_moving_right[] = {
    LIBRARY_IMAGE_ARISTO_RIGHT_1,
    LIBRARY_IMAGE_ARISTO_RIGHT_2,
    LIBRARY_IMAGE_ARISTO_RIGHT_1,
    LIBRARY_IMAGE_ARISTO_RIGHT_3,
};

static constexpr int aristocrate_moving_up[] = {
    LIBRARY_IMAGE_ARISTO_UP_1,
    LIBRARY_IMAGE_ARISTO_UP_2,
};

} // namespace rpg::animation

namespace rpg::dialog
{

static const char *aristocrate_dialog_1[] = {
    "It is a good life\n",
    "we live, brother.\n\n",
    "The best. And may\n",
    "it never change.",
    "",
};

static const char *aristocrate_dialog_2[] = {
    "Nothing is true,\n", "everything is\n", "permitted.", ""};

static const char *aristocrate_dialog_3[] = {
    "Database, database,\n",
    "just living in the\n",
    "Database.",
    "",
};

} // namespace rpg::dialog

namespace rpg
{

class CharacterAristocrate : virtual public Character, public MovingMixin
{
  public:
    CharacterAristocrate()
        : Character(SceneObjectIdentity::GENERIC, 24, 28, 11, 26)
    {
        set_animation_variant(Appearance::moving_down,
                              animation::aristocrate_moving_down, 2);

        set_animation_variant(Appearance::moving_left,
                              animation::aristocrate_moving_left, 4);

        set_animation_variant(Appearance::moving_right,
                              animation::aristocrate_moving_right, 4);

        set_animation_variant(Appearance::moving_up,
                              animation::aristocrate_moving_up, 2);
    }

    virtual const char **get_dialog() override
    {
        if (next_dialog == dialog::aristocrate_dialog_1) {
            next_dialog = dialog::aristocrate_dialog_2;
            return dialog::aristocrate_dialog_1;
        }

        if (next_dialog == dialog::aristocrate_dialog_2) {
            next_dialog = dialog::aristocrate_dialog_3;
            return dialog::aristocrate_dialog_2;
        }

        if (next_dialog == dialog::aristocrate_dialog_3) {
            next_dialog = dialog::aristocrate_dialog_1;
            return dialog::aristocrate_dialog_3;
        }

        next_dialog = dialog::aristocrate_dialog_2;
        return dialog::aristocrate_dialog_1;
    }

    virtual const char *get_name() const override
    {
        return "Guildmaster";
    }

    virtual void post_render(Viewport &viewport) override;
    virtual void render(Viewport &viewport) override;

  private:
    enum class Mode {
        standing,
        walking,
    };

    Mode current_mode = Mode::standing;
    const char **next_dialog;
};

} // namespace rpg
