#include "FX.h"
#include "esp_console.h"
#include "esp_spiffs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "FastLED.h"
#include "buttons.h"
#include "cmd.h"
#include "graphics.h"
#include "infoscreen.h"
#include "neopixel.h"
#include "rpg.h"

static bool mount_spiffs()
{
    esp_vfs_spiffs_conf_t conf = {.base_path = "/spiffs",
                                  .partition_label = NULL,
                                  .max_files = 8,
                                  .format_if_mount_failed = false};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    return ret == ESP_OK;
}

static void welcome_screen_task(void *arg)
{
    graphics_draw_jpeg("/spiffs/welcome/blank.jpeg", 0, 0);
    graphics_update_display();
    vTaskDelay(750 / portTICK_PERIOD_MS);

    graphics_draw_jpeg("/spiffs/welcome/nsec2021.jpeg", 0, 29);
    graphics_update_display();
    vTaskDelay(750 / portTICK_PERIOD_MS);

    int sponsored_y = 183;
    for (int fade = 0; fade <= 100; ++fade) {
        switch (fade) {
        case 0:
            graphics_draw_jpeg("/spiffs/welcome/sponsored-0.jpeg", 0,
                               sponsored_y);
            break;

        case 1:
            graphics_draw_jpeg("/spiffs/welcome/sponsored-1.jpeg", 0,
                               sponsored_y);
            break;

        case 2:
            graphics_draw_jpeg("/spiffs/welcome/sponsored-2.jpeg", 0,
                               sponsored_y);
            break;

        case 4:
            graphics_draw_jpeg("/spiffs/welcome/sponsored-4.jpeg", 0,
                               sponsored_y);
            break;

        case 8:
            graphics_draw_jpeg("/spiffs/welcome/sponsored-8.jpeg", 0,
                               sponsored_y);
            break;

        case 16:
            graphics_draw_jpeg("/spiffs/welcome/sponsored-16.jpeg", 0,
                               sponsored_y);
            break;

        case 32:
            graphics_draw_jpeg("/spiffs/welcome/sponsored-32.jpeg", 0,
                               sponsored_y);
            break;

        case 64:
            graphics_draw_jpeg("/spiffs/welcome/sponsored-64.jpeg", 0,
                               sponsored_y);
            break;

        case 80:
            graphics_draw_jpeg("/spiffs/welcome/sponsored-80.jpeg", 0,
                               sponsored_y);
            break;

        case 90:
            graphics_draw_jpeg("/spiffs/welcome/sponsored-90.jpeg", 0,
                               sponsored_y);
            break;

        case 100:
            graphics_draw_jpeg("/spiffs/welcome/sponsored-100.jpeg", 0,
                               sponsored_y);
            break;

        default:
            continue;
        }

        graphics_update_display();
    }

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    for (int i = 0; i < 66; i += 7) {
        graphics_fadeout_display_buffer(i);
        graphics_update_display();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    vTaskDelete(NULL);
}

extern "C" void app_main(void)
{
    button_t button = BUTTON_NONE;

    if (!mount_spiffs())
        abort();

    graphics_start();
    initialize_nvs();
    nsec_buttons_init();
    NeoPixel::getInstance().setMode(FX_MODE_RAINBOW);

    TaskHandle_t welcome_task;
    xTaskCreate(welcome_screen_task, NULL, 2048, NULL, 1, &welcome_task);
    xTaskCreate(console_task, "console task", 4096, NULL, 3, NULL);

    while (eTaskGetState(welcome_task) != eDeleted) {
        xQueueReceive(button_event_queue, &button, 0);

        // Make it possible to skip the welcome screen.
        if (button == BUTTON_BACK_RELEASE || button == BUTTON_ENTER_RELEASE) {
            vTaskDelete(welcome_task);

            while (eTaskGetState(welcome_task) != eDeleted)
                vTaskDelay(1);
        }

        vTaskDelay(1);
    }

    infoscreen_display_bootwarning();

    for (int i = 100; i >= 0; i -= 25) {
        graphics_draw_jpeg("/spiffs/welcome/start.jpeg", 0, 0);
        graphics_fadeout_display_buffer(i);
        graphics_update_display();
    }

    do {
        vTaskDelay(1);
        xQueueReceive(button_event_queue, &button, 0);
    } while (button != BUTTON_ENTER_RELEASE);

    run_main_scene();
}
