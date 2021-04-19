#include "save.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdio.h>

#define STORAGE_NAMESPACE "storage"

SaveData Save::save_data = {
    .buzzer_enable_music = false,
    .buzzer_enable_music_id = 1,
    .buzzer_enable_sfx = true,
    .buzzer_enable_steps = true,
    .chest_opened_island = false,
    .chest_opened_konami = false,
    .chest_opened_welcome = false,
    .konami_done = false,
    .neopixel_brightness = 25,
    .neopixel_mode = 0,
    .neopixel_is_on = true,
    .neopixel_color = 0xff00ff,
    .flag1 = false,
    .flag2 = false,
    .flag3 = false,
    .flag4 = false,
    .flag5 = false,
    .flag6 = false,
    .flag7 = false,
    .flag8 = false,
    .flag9 = false,
    .flag10 = false,
    .flag11 = false,
    .flag12 = false,
    .flag13 = false,
    .flag14 = false,
    .flag15 = false,
};

esp_err_t Save::write_save()
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_blob(my_handle, "save", reinterpret_cast<void *>(&save_data),
                       sizeof(save_data));

    if (err != ESP_OK) {
        return err;
    }

    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        return err;
    }

    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t Save::load_save()
{
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("save", "nvs open save failed");
        return err;
    }

    size_t save_size = sizeof(save_data);
    err = nvs_get_blob(my_handle, "save", reinterpret_cast<void *>(&save_data),
                       &save_size);
    if (err != ESP_OK) {
        return err;
    }

    nvs_close(my_handle);
    return ESP_OK;
}
