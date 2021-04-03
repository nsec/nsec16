#pragma once

#include "rpg/Character.h"

#include "graphics.h"

namespace rpg::animation
{

static constexpr int charles_standing[] = {
    LIBRARY_IMAGE_CHARLES_1,
    LIBRARY_IMAGE_CHARLES_2,
};

} // namespace rpg::animation

namespace rpg::dialog
{

static const char *charles_dialog_1[] = {
    "I like to wear\n",
    "trousers. They're\n",
    "comfy and easy\n"
    "to wear.",
    "",
};

} // namespace rgp::dialog

namespace rpg
{

class CharacterCharles : public Character
{
  public:
    CharacterCharles() : Character(character::Identity::GENERIC, 15, 34, 8, 33)
    {
        set_animation_variant(Appearance::standing, animation::charles_standing,
                              2);
    }

    virtual const char **get_dialog() override
    {
        return dialog::charles_dialog_1;
    }

    virtual const char *get_name() const override { return "Charles"; }

    virtual void render(Viewport &viewport) override;
};

} // namespace rpg
