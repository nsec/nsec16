#pragma once

#include "rpg/Character.h"

#include "graphics.h"

namespace rpg::animation
{

static constexpr int fisherman_standing[] = {
    LIBRARY_IMAGE_FISHERMAN_1, LIBRARY_IMAGE_FISHERMAN_1,
    LIBRARY_IMAGE_FISHERMAN_1, LIBRARY_IMAGE_FISHERMAN_1,
    LIBRARY_IMAGE_FISHERMAN_2, LIBRARY_IMAGE_FISHERMAN_3,
    LIBRARY_IMAGE_FISHERMAN_1, LIBRARY_IMAGE_FISHERMAN_4,
};

} // namespace rpg::animation

namespace rpg::dialog
{

static const char *fisherman_dialog_1[] = {
    "I heard you were\n", "coming.\n", "\n", "Want to go phishing?\n", "",
};

static const char *fisherman_dialog_2[] = {
    "Do you think there\n",
    "is more fish on\n",
    "the oposite shore?\n",
    "",
};

static const char *fisherman_dialog_3[] = {
    "I got the Babel\n",
    "Fish.",
    "",
};

} // namespace rpg::dialog

namespace rpg
{

class CharacterFisherman : public Character
{
  public:
    CharacterFisherman()
        : Character(character::Identity::GENERIC, 56, 27, 9, 24)
    {
        set_animation_variant(Appearance::standing,
                              animation::fisherman_standing, 8);
    }

    virtual const char **get_dialog() override
    {
        switch (get_animation_step() % 3) {
        case 0:
            return dialog::fisherman_dialog_1;

        case 1:
            return dialog::fisherman_dialog_2;

        case 2:
        default:
            return dialog::fisherman_dialog_3;
        }
    }

    virtual const char *get_name() const override
    {
        return "fisherman";
    }

    virtual void render(Viewport &viewport) override;
};

} // namespace rpg
