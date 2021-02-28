#pragma once

namespace rpg
{

constexpr int viewport_width = 264;
constexpr int viewport_tiles_width = 11;
constexpr int viewport_height = 234;
constexpr int viewport_tiles_height = 10;

constexpr int viewport_prepend_cols = 2;
constexpr int viewport_prepend_rows = 2;

constexpr int viewport_crop_width = 240;
constexpr int viewport_crop_height = 210;

} // namespace rpg

namespace rpg::data
{

constexpr int tilemap_cell_extra = 2;
constexpr int tilemap_cell_words = 10;

constexpr int tilemap_line_words =
    tilemap_cell_words * (rpg::viewport_tiles_width + tilemap_cell_extra);

constexpr int tilemap_read_lines =
    rpg::viewport_tiles_height + rpg::viewport_prepend_rows;

constexpr int tilemap_read_lines_extra = 2;

} // namespace rpg::data