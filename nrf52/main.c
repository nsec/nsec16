//  Copyright (c) 2017
//  Benjamin Vanheuverzwijn <bvanheu@gmail.com>
//  Marc-Etienne M. Leveille <marc.etienne.ml@gmail.com>
//
//  License: MIT (see LICENSE for details)


#include "ble/nsec_ble.h"

#include <nrf.h>
#include <nrf_error.h>
#include <nrf_gpio.h>
#include <nrf_delay.h>
#include <nrf52.h>
#include <nrf52_bitfields.h>
#include <nordic_common.h>
#include <stdint.h>
#include <nrf_soc.h>

#include "ble/ble_device.h"
#include "battery.h"

#include "buttons.h"
#include "logs.h"
#include "gfx_effect.h"
#include "identity.h"
#include "menu.h"
#include "nsec_conf_schedule.h"
#include "nsec_settings.h"
#include "timer.h"
#include "power.h"
#include "softdevice.h"
#include "status_bar.h"
#include "ssd1306.h"
#include "utils.h"
#include "ws2812fx.h"

#include "images/nsec_logo_bitmap.c"
#include "ble/service_characteristic.h"
#include "ble/vendor_service.h"

static char g_device_id[10];
bool is_at_main_menu = false;

/*
 * Callback function when APP_ERROR_CHECK() fails
 *
 *  - Display the filename, line number and error code.
 *  - Flash all neopixels red for 5 seconds.
 *  - Reset the system.
 */
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info) {
    static int error_displayed = 0;
    uint8_t count = 50;
    if(!error_displayed) {
        char error_msg[128];
        error_info_t *err_info = (error_info_t *) info;
        snprintf(error_msg, sizeof(error_msg), "%s:%u -> error 0x%08x\r\n",
                 err_info->p_file_name, (unsigned int)err_info->line_num,
                 (unsigned int)err_info->err_code);
        puts(error_msg);
        gfx_setCursor(0, 0);
        gfx_puts(error_msg);
        gfx_update();
        error_displayed = 1;
    }

    init_WS2812FX();
    setBrightness_WS2812FX(64);
    setSpeed_WS2812FX(200);
    setColor_WS2812FX(255, 0, 0);
    setMode_WS2812FX(FX_MODE_BLINK);
    start_WS2812FX();

    while (count > 0) {
        service_WS2812FX();
        nrf_delay_ms(100);
        count--;
    }

    NVIC_SystemReset();
}

static
void nsec_intro(void) {
    for(uint8_t noise = 128; noise <= 128; noise -= 8) {
        gfx_fillScreen(SSD1306_BLACK);
        gfx_drawBitmap(17, 11, nsec_logo_bitmap, nsec_logo_bitmap_width, nsec_logo_bitmap_height, SSD1306_WHITE);
        nsec_gfx_effect_addNoise(noise);
        gfx_update();
    }
    for(uint8_t noise = 0; noise <= 128; noise += 8) {
        gfx_fillScreen(SSD1306_BLACK);
        gfx_drawBitmap(17, 11, nsec_logo_bitmap, nsec_logo_bitmap_width, nsec_logo_bitmap_height, SSD1306_WHITE);
        nsec_gfx_effect_addNoise(noise);
        gfx_update();
    }
}

void open_conference_schedule(uint8_t item) {
    menu_close();
    is_at_main_menu = false;
    nsec_schedule_show_dates();
}

void open_settings(uint8_t item) {
    menu_close();
    is_at_main_menu = false;
    nsec_setting_show();
}

static
menu_item_s main_menu_items[] = {
    {
        .label = "Conference schedule",
        .handler = open_conference_schedule,
    }, {
        .label = "Settings",
        .handler = open_settings,
    }
};

void show_main_menu(void) {
    for(uint8_t noise = 128; noise <= 128; noise -= 16) {
        gfx_fillScreen(SSD1306_BLACK);
        nsec_identity_draw();
        nsec_gfx_effect_addNoise(noise);
        gfx_update();
    }
    nsec_status_bar_ui_redraw();
    menu_init(0, 64-8, 128, 8, sizeof(main_menu_items) / sizeof(main_menu_items[0]), main_menu_items);
    is_at_main_menu = true;
}

uint16_t service0_write_event_handler(CharacteristicWriteEvent* event){
	NRF_LOG_INFO("Write request, I will not allow it.");
	return BLE_GATT_STATUS_ATTERR_WRITE_NOT_PERMITTED;
}

uint16_t service0_read_request_handler(CharacteristicReadEvent* event){
	NRF_LOG_INFO("Read request, I will allow it.");
	return BLE_GATT_STATUS_SUCCESS;
}

void service1_write_event_handler(CharacteristicWriteEvent* event){
	NRF_LOG_INFO("Write command for service 1");
}

int main(void) {
    sprintf(g_device_id, "NSEC%04X", (uint16_t)(NRF_FICR->DEVICEID[1] % 0xFFFF));
    g_device_id[9] = '\0';

    /*
     * Initialize base hardware
     */
    log_init();
    power_init();
    softdevice_init();
	battery_init();

    /*
     * Initialize bluetooth stack
     */
    create_ble_device("My BLE device");
    configure_advertising();
    VendorService service0;
    ServiceCharacteristic characteristic0;
    create_vendor_service(&service0);
    config_dummy_service(&service0, &characteristic0);
    uint8_t value = 0xab;
    set_characteristic_value(&characteristic0, &value);
    add_write_request_handler(&characteristic0, service0_write_event_handler);
    add_read_request_handler(&characteristic0, service0_read_request_handler);

    VendorService service1;
	create_vendor_service(&service1);
	add_vendor_service(&service1);
	ServiceCharacteristic characteristic1;
	add_characteristic_to_vendor_service(&service1, &characteristic1, 1, true, true, true, true);
	add_write_event_handler(&characteristic1, service1_write_event_handler);
	value = 0xba;
	set_characteristic_value(&characteristic1, &value);
    start_advertising();
    nsec_identity_init();
    nsec_status_bar_init();
    nsec_status_set_name(g_device_id);
    nsec_status_set_badge_class(NSEC_STRINGIFY(NSEC_HARDCODED_BADGE_CLASS));
    nsec_status_set_ble_status(STATUS_BLUETOOTH_ON);

    setBrightness_WS2812FX(64);
    setSpeed_WS2812FX(200);
    setMode_WS2812FX(FX_MODE_RAINBOW_CYCLE);
    start_WS2812FX();

    nsec_intro();
    show_main_menu();

    nsec_identity_draw();

    /*
     * Main loop
     */
    while(true) {
        service_WS2812FX();
        nrf_delay_ms(50);
    }

    return 0;
}
