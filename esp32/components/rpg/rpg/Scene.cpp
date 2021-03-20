#include "rpg/Scene.h"

#include "graphics.h"

namespace rpg
{

void Scene::add_character(Character *character)
{
    characters.push_back(character);
    scene_objects.push_back(character);
}

void Scene::add_character(MainCharacter *character)
{
    assert(
        main_character == nullptr &&
        "Cannot set pointer to the main character because it is already set.");

    main_character = character;
    add_character(static_cast<Character *>(character));
}

/**
 * Find all tiles that should be treated as objects.
 *
 * The current implementation of the rendering loop treats level 4 differently
 * from the rest - it is assumed to only contain objects that the main
 * character can interact with in some way. All image tiles in level 4 must be
 * converted to an object that captures all information necessary for correct
 * rendering, which is done by this method.
 *
 * The tilemap scan is performed in the blocks of the size of the viewport in
 * order to reduce the number of read operations from the flash.
 */
void Scene::populate_objects_from_scene()
{
    for (int x_a = 0; x_a < scene_size.tile_x(); x_a += viewport_tiles_width) {
        for (int y_a = 0; y_a < scene_size.tile_y();
             y_a += viewport_tiles_height) {

            for (int x_b = 0; x_b < viewport_tiles_width; ++x_b) {
                for (int y_b = 0; y_b < viewport_tiles_height; ++y_b) {
                    if (x_a + x_b >= scene_size.tile_x())
                        continue;

                    if (y_a + y_b >= scene_size.tile_y())
                        continue;

                    auto coordinates =
                        GlobalCoordinates::tile(x_a + x_b, y_a + y_b);

                    int image = data_reader.get_image(coordinates, 4);
                    if (image) {
                        // No bounds checking, YOLO.
                        int height =
                            graphics_static_images_registry[image].height;

                        int sinkline =
                            graphics_static_images_registry[image].sinkline;

                        int width =
                            graphics_static_images_registry[image].width;

                        // Because the raw pointers to created objects are
                        // saved into the scene_objects vector, if the
                        // tile_objects vector re-allocates its internal
                        // storage, all those pointers will become invalid.
                        assert(tile_objects.size() <
                                   tile_objects_vector_reserve - 1 &&
                               "Adding any more objects to tile_objects vector "
                               "will cause it to re-allocate.");

                        tile_objects.emplace_back(coordinates, width, height,
                                                  image, sinkline);

                        scene_objects.push_back(&(tile_objects.back()));
                    }
                }
            }
        }
    }
}

void Scene::render()
{
    if (!lock())
        return;

    int character_sinkline =
        viewport.to_screen(main_character->get_coordinates()).y() +
        main_character->get_ground_base_y();

    graphics_clip_set(0, 0, viewport_crop_width, viewport_crop_height);

    viewport.prime_refresh_state(characters);

    for (int layer = 0; layer < 4; ++layer)
        render_layer(layer, 0, false);

    bool repeat = render_layer(4, character_sinkline, false);

    for (auto character : characters)
        character->render(viewport);

    if (repeat)
        render_layer(4, character_sinkline, true);

    for (int layer = 5; layer < 8; ++layer)
        render_layer(layer, 0, false);

    graphics_update_display();
    graphics_clip_reset();
    viewport.cache_refresh_state();
    unlock();
}

bool Scene::render_layer(int layer, int sinkline_check, bool sinkline_repeat)
{
    bool repeat = false;
    int sinkline = 0;

    data::tilemap_word_t dependency;
    data::tilemap_word_t image;

    for (int y = -viewport_prepend_rows; y < 0; ++y) {
        for (int x = -viewport_prepend_cols; x < viewport_tiles_width; ++x) {
            auto local = LocalCoordinates::tile(x, y);
            auto global = viewport.to_global(local);

            dependency = data_reader.get_dependency(global);
            if (dependency == 0)
                continue;

            image = data_reader.get_image(global, layer);
            if (image == 0)
                continue;

            if (!viewport.tile_needs_refresh(local))
                continue;

            auto screen = viewport.to_screen(local);
            graphics_draw_from_library(image, screen.x(), screen.y());
        }
    }

    for (int y = 0; y < viewport_tiles_height; ++y) {
        for (int x = -viewport_prepend_cols; x < 0; ++x) {
            auto local = LocalCoordinates::tile(x, y);
            auto global = viewport.to_global(local);

            dependency = data_reader.get_dependency(global);
            if (dependency == 0)
                continue;

            image = data_reader.get_image(global, layer);
            if (image == 0)
                continue;

            if (!viewport.tile_needs_refresh(local))
                continue;

            auto screen = viewport.to_screen(local);
            graphics_draw_from_library(image, screen.x(), screen.y());
        }

        for (int x = 0; x < viewport_tiles_width; ++x) {
            auto local = LocalCoordinates::tile(x, y);
            auto global = viewport.to_global(local);

            image = data_reader.get_image(global, layer);
            if (image == 0)
                continue;

            if (!viewport.tile_needs_refresh(local))
                continue;

            auto screen = viewport.to_screen(local);

            if (sinkline_check) {
                sinkline =
                    screen.y() + graphics_get_sinkline_from_library(image);

                if (sinkline >= sinkline_check && !sinkline_repeat) {
                    repeat = true;
                    continue;
                }

                if (sinkline < sinkline_check && sinkline_repeat) {
                    continue;
                }
            }

            graphics_draw_from_library(image, screen.x(), screen.y());
        }
    }

    return repeat;
}

} // namespace rpg
