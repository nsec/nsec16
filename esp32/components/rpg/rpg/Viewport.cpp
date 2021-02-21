#include "rpg/Viewport.h"

namespace rpg
{

static inline unsigned int refresh_state_tile_offset(int tile_x, int tile_y)
{
    return (viewport_prepend_rows + tile_y) *
               (viewport_prepend_cols + viewport_tiles_width) +
           viewport_prepend_cols + tile_x;
}

void Viewport::cache_refresh_state()
{
    for (auto &i : *refresh_state)
        i = false;

    needs_full_refresh = false;
}

local_coordinates_t Viewport::get_local_coordinates(int scene_x, int scene_y)
{
    int screen_x = scene_x - x;
    int screen_y = scene_y - y;

    int local_tile_x = screen_x / DISPLAY_TILE_WIDTH;
    if (screen_x < 0)
        local_tile_x--;

    int local_tile_y = screen_y / DISPLAY_TILE_HEIGHT;
    if (screen_y < 0)
        local_tile_y--;

    return {
        local_tile_x = local_tile_x,
        local_tile_y = local_tile_y,
        screen_x = screen_x,
        screen_y = screen_y,
    };
}

tile_coordinates_t Viewport::get_tile_coordinates(int local_tile_x,
                                                  int local_tile_y)
{
    return {
        .screen_x = (local_tile_x * DISPLAY_TILE_WIDTH),
        .screen_y = (local_tile_y * DISPLAY_TILE_HEIGHT),
        .tile_x = local_tile_x + tile_x,
        .tile_y = local_tile_y + tile_y,
    };
}

void Viewport::move_to_tile(int new_tile_x, int new_tile_y)
{
    if (new_tile_x >= 0 && new_tile_x < scene_width_tiles) {
        tile_x = new_tile_x;
        x = new_tile_x * DISPLAY_TILE_WIDTH;
    }

    if (new_tile_y >= 0 && new_tile_y < scene_height_tiles) {
        tile_y = new_tile_y;
        y = new_tile_y * DISPLAY_TILE_HEIGHT;
    }

    needs_full_refresh = true;
}

void Viewport::prime_refresh_state(data::SceneDataReader &data_reader,
                                   const std::vector<Character *> &characters)
{
    if (needs_full_refresh) {
        for (auto &i : *refresh_state)
            i = true;

        return;
    }

    for (auto character_p : characters) {
        local_coordinates_t coordinates_array[] = {
            get_local_coordinates(character_p->get_rendered_scene_x(),
                                  character_p->get_rendered_scene_y()),
            get_local_coordinates(character_p->get_scene_x(),
                                  character_p->get_scene_y()),
        };

        for (auto &coordinates : coordinates_array) {
            for (int dy = 0, y_limit = coordinates.screen_y +
                                       character_p->get_height() +
                                       DISPLAY_TILE_HEIGHT;
                 coordinates.screen_y < y_limit;
                 ++dy, coordinates.screen_y += DISPLAY_TILE_HEIGHT) {

                for (int dx = 0, x_limit = coordinates.screen_x +
                                           character_p->get_width() +
                                           DISPLAY_TILE_WIDTH;
                     coordinates.screen_x < x_limit;
                     ++dx, coordinates.screen_x += DISPLAY_TILE_WIDTH) {

                    prime_refresh_state_tile(data_reader,
                                             coordinates.local_tile_x + dx,
                                             coordinates.local_tile_y + dy);
                }
            }
        }
    }
}

void Viewport::prime_refresh_state_tile(data::SceneDataReader &data_reader,
                                        int tile_x, int tile_y)
{
    if (tile_x < -viewport_prepend_cols || tile_y < -viewport_prepend_rows)
        return;

    if (tile_x >= viewport_tiles_width || tile_y >= viewport_tiles_height)
        return;

    unsigned int offset = refresh_state_tile_offset(tile_x, tile_y);
    if ((*refresh_state)[offset])
        return;

    (*refresh_state)[offset] = true;

    data::tilemap_word_t dependency =
        data_reader.get_dependency(tile_x, tile_y);

    if (dependency != 0) {
        data::tilemap_dependency_t decoded_dependency =
            data_reader.decode_dependency(dependency);

        for (int dy = decoded_dependency.backward_y; dy > -1; --dy) {
            for (int dx = decoded_dependency.backward_x; dx > -1; --dx) {
                prime_refresh_state_tile(data_reader, tile_x - dx, tile_y - dy);
            }
        }

        for (int dy = 0; dy <= decoded_dependency.forward_y; ++dy) {
            for (int dx = 0; dx <= decoded_dependency.forward_x; ++dx) {
                prime_refresh_state_tile(data_reader, tile_x + dx, tile_y + dy);
            }
        }
    }
}

bool Viewport::tile_needs_refresh(int tile_x, int tile_y) const
{
    if (tile_x < -viewport_prepend_cols || tile_y < -viewport_prepend_rows)
        return false;

    if (tile_x >= viewport_tiles_width || tile_y >= viewport_tiles_height)
        return false;

    return (*refresh_state)[refresh_state_tile_offset(tile_x, tile_y)];
}

} // namespace rpg
