#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_console.h"

#include "buttons.h"
#include "graphics.h"
#include "rpg.h"
#include "cmd.h"

extern "C" void app_main(void)
{
    xTaskCreate(init_cmd, "repl task", 4096, NULL, 3, NULL);
    nsec_buttons_init();
    graphics_start();

    run_main_scene();
}
