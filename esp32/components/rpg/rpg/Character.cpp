#include "rpg/Character.h"

#include "rpg/Viewport.h"

namespace rpg
{

bool Character::is_visible(Viewport &viewport)
{
    auto coordinates = GlobalCoordinates::xy(scene_x, scene_y);
    if (viewport.is_visible(coordinates))
        return true;

    coordinates.change_xy_by(width, 0);
    if (viewport.is_visible(coordinates))
        return true;

    coordinates.change_xy_by(0, height);
    if (viewport.is_visible(coordinates))
        return true;

    coordinates.change_xy_by(-width, 0);
    if (viewport.is_visible(coordinates))
        return true;

    return false;
}

bool Character::move(GlobalCoordinates coordinates)
{
    if (blocked_data_reader) {
        auto ground = coordinates;
        ground.change_xy_by(get_ground_base_x(), get_ground_base_y());

        if (blocked_data_reader->is_blocked(ground))
            return false;
    }

    move_distance = coordinates;
    move_distance.change_xy_by(-scene_x, -scene_y);

    scene_x = coordinates.x();
    scene_y = coordinates.y();

    return true;
}

void Character::render(Viewport &viewport)
{
    rendered_scene_x = scene_x;
    rendered_scene_y = scene_y;
}

void Character::render_animation_variant(Viewport &viewport,
                                         const Appearance appearance,
                                         const int slow_down)
{
    const int *sprites;
    int count;

    if (appearance == Appearance::LENGTH)
        return;

    sprites = animation_variants[static_cast<int>(appearance)].sprites;
    count = animation_variants[static_cast<int>(appearance)].count;

    if (sprites == nullptr || count < 1)
        return;

    int index = (get_animation_step() / slow_down) % count;
    auto screen = viewport.to_screen(get_coordinates());

    graphics_draw_from_library(sprites[index], screen.x(), screen.y());
}

void Character::set_animation_variant(const Appearance appearance,
                                      const int *sprites, const int count)
{
    switch (appearance) {
    case Appearance::LENGTH:
        break;

    default:
        animation_variants[static_cast<int>(appearance)].sprites = sprites;
        animation_variants[static_cast<int>(appearance)].count = count;
    }
}

} // namespace rpg
