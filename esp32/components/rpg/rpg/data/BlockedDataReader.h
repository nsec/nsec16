#pragma once

#include "rpg_const.h"

extern "C" {
#include "graphics.h"
}

#include "esp_log.h"

#include <string>

namespace rpg::data
{

class BlockedDataReader
{
  public:
    BlockedDataReader(const char *scene_name, int scene_width, int scene_height)
        : scene_width{scene_width}, scene_height{scene_height}
    {
        std::string filename{"/spiffs/rpg/"};
        filename += scene_name;
        filename += ".blocked";

        FILE *file = fopen(filename.c_str(), "r");
        if (!file) {
            ESP_LOGE(__FUNCTION__,
                     "Scene blocked area cannot be read from SPIFFS: %s",
                     filename.c_str());
            abort();
        }

        line_words = scene_width / (2 * DISPLAY_TILE_WIDTH);
        int read_size = line_words * (scene_height / 6);

        blocked_data =
            static_cast<uint8_t *>(calloc(read_size, sizeof(uint8_t)));

        if (!blocked_data) {
            ESP_LOGE(__FUNCTION__, "Failed to allocate blocked_data.");
            abort();
        }

        fseek(file, 0, SEEK_SET);
        fread(blocked_data, sizeof(uint8_t), read_size, file);
        fclose(file);
    }

    ~BlockedDataReader()
    {
        delete[] blocked_data;
    }

    bool is_blocked(int x, int y);

  private:
    uint8_t *blocked_data;

    int scene_width;
    int scene_height;

    unsigned int line_words = 0;
};

} // namespace rpg::data