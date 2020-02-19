//  Copyright (c) 2017
//  Benjamin Vanheuverzwijn <bvanheu@gmail.com>
//  Marc-Etienne M. Leveille <marc.etienne.ml@gmail.com>
//
//  License: MIT (see LICENSE for details)

#include "nsec_settings.h"
#include "FreeRTOS.h"
#include "ble/ble_device.h"
#include "drivers/buttons.h"
#include "drivers/controls.h"
#include "drivers/display.h"
#include "drivers/ws2812fx.h"
#include "gfx_effect.h"
#include "gui.h"
#include "home_menu.h"
#include "main_menu.h"
#include "menu.h"
#include "nrf_delay.h"
#include "nsec_led_settings.h"
#include "nsec_screen_settings.h"
#include "persistency.h"
#include "queue.h"
#include "status_bar.h"
#include "timer.h"
#include <nordic_common.h>
#include <nrf.h>
#include <stdio.h>
#include <string.h>

static void toggle_bluetooth(uint8_t item);
static void show_credit(uint8_t item);
static void turn_off_screen(uint8_t item);
static void show_led_settings(uint8_t item);
static void show_screen_settings(uint8_t item);
static void confirm_factory_reset(uint8_t item);
static void do_factory_reset(uint8_t item);
static void show_member_details(uint8_t item);

enum setting_state {
    SETTING_STATE_CLOSED,
    SETTING_STATE_MENU,
    SETTING_STATE_CREDIT,
    SETTING_STATE_CREDIT_DETAILS,
    SETTING_STATE_SCREEN_OFF,
    SETTING_STATE_BATTERY,
    SETTING_CONFIRM_FACTORY
};

static enum setting_state _state = SETTING_STATE_CLOSED;

static const menu_item_s settings_items[] = {
    {
        .label = "Led settings",
        .handler = show_led_settings,
    },
    {
        .label = "Screen settings",
        .handler = show_screen_settings,
    },
    {
        .label = "Factory Reset",
        .handler = do_factory_reset,
    },
    {
        .label = "Toggle Bluetooth",
        .handler = toggle_bluetooth,
    },
    {
        .label = "Turn screen off",
        .handler = turn_off_screen,
    },
    {
        .label = "Credit",
        .handler = show_credit,
    },
};

static const menu_item_s members_items[] = {
    {
        .label = "Eric Tremblay",
        .handler = show_member_details,
    },
    {
        .label = "Michael Jeanson",
        .handler = show_member_details,
    },
    {
        .label = "Nicolas Aubry",
        .handler = show_member_details,
    },
    {
        .label = "Simon Marchi",
        .handler = show_member_details,
    },
    {
        .label = "Thomas Dupuy",
        .handler = show_member_details,
    },
    {
        .label = "Victor Nikulshin",
        .handler = show_member_details,
    },
    {
        .label = "Yannick Lamarre",
        .handler = show_member_details,
    },
};

static const menu_item_s confirm_items[] = {
    {
        .label = "Yes",
        .handler = confirm_factory_reset,
    },
    {
        .label = "No",
        .handler = confirm_factory_reset,
    },
};

static menu_t g_menu;

static void redraw_settings_menu(menu_t *menu)
{
    draw_settings_title();
    menu_ui_redraw_all(menu);
}

static void confirm_factory_reset(uint8_t item)
{
    gfx_fill_rect(GEN_MENU_POS, GEN_MENU_WIDTH, GEN_MENU_HEIGHT, DISPLAY_WHITE);
    gfx_set_cursor(GEN_MENU_POS);
    gfx_set_text_background_color(HOME_MENU_BG_COLOR, DISPLAY_WHITE);

    if (!item) {
        set_default_persistency();
        gfx_puts("Settings restored to\ndefault rebooting...");

        // Not neccessary but it make it more... serious ???
        nrf_delay_ms(2000);
        NVIC_SystemReset();
    } else {
        gfx_puts("Okay...");
        nrf_delay_ms(1000);
        nsec_setting_show();
    }
}

static void do_factory_reset(uint8_t item)
{
    gfx_fill_rect(GEN_MENU_POS, GEN_MENU_WIDTH, GEN_MENU_HEIGHT, DISPLAY_WHITE);
    gfx_set_cursor(GEN_MENU_POS);
    gfx_set_text_background_color(HOME_MENU_BG_COLOR, DISPLAY_WHITE);
    gfx_puts("Are you sure ?");

    menu_init(&g_menu, GEN_MENU_POS_X, GEN_MENU_POS_Y + 16, GEN_MENU_WIDTH,
              GEN_MENU_HEIGHT, ARRAY_SIZE(confirm_items), confirm_items,
              HOME_MENU_BG_COLOR, DISPLAY_WHITE);

    _state = SETTING_CONFIRM_FACTORY;
}

static void show_member_details(uint8_t item) {
    gfx_fill_rect(GEN_MENU_POS, GEN_MENU_WIDTH, GEN_MENU_HEIGHT, DISPLAY_WHITE);
    gfx_set_cursor(GEN_MENU_POS);
    gfx_set_text_background_color(HOME_MENU_BG_COLOR, DISPLAY_WHITE);
    _state = SETTING_STATE_CREDIT_DETAILS;

    switch (item) {
        //Line   |                     | 21 character
        // There are 7 lines under the title
        case 0: //Eric Tremblay
        gfx_puts("VP Badge nsec 2019\n");
        gfx_puts("habscup@gmail.com\n");
        gfx_puts("Hardware\n");
        gfx_puts("Software\n");
        break;

        case 1: //Michael Jeanson
        gfx_puts("Badge drone nsec 2019\n");
        gfx_puts("Michael Jeanson\n");
        gfx_puts("mjeanson@gmail.com\n");
        gfx_puts("@mjeanson\n");
        gfx_puts("Software\n");
        break;

        case 2: // Nicolas Aubry
        gfx_puts("Nicolas Aubry\n");
        gfx_puts("Software\n");
        break;

        case 3: //Simon Marchi
            gfx_puts("C monkey nsec 2019\n");
            gfx_puts("Simon Marchi\n");
            gfx_puts("simark@simark.ca\n");
            gfx_puts("@simark\n");
            gfx_puts("Software\n");
            break;

        case 4: //Thomas Dupuy
        show_credit(4);
        break;

        case 5: //Victor Nikulshin
        show_credit(4);
        break;

        case 6: //Yannick Lamarre
        show_credit(4);
        break;
    }
    gfx_update();
}

static void show_led_settings(uint8_t item) {
    _state = SETTING_STATE_CLOSED;
    nsec_show_led_settings();
    redraw_settings_menu(&g_menu);
}

static void show_screen_settings(uint8_t item) {
    _state = SETTING_STATE_CLOSED;
    nsec_show_screen_settings();
}

static void toggle_bluetooth(uint8_t item) {
//    if(ble_device_toggle_ble()) {
//        nsec_status_set_ble_status(STATUS_BLUETOOTH_ON);
//    }
//    else {
//        nsec_status_set_ble_status(STATUS_BLUETOOTH_OFF);
//    }
}

static void draw_credit_title(void)
{
    draw_title("CREDIT", 25, 5, DISPLAY_BLUE, DISPLAY_WHITE);
}

static void show_credit(uint8_t item) {
    _state = SETTING_STATE_CREDIT;
    gfx_fill_rect(GEN_MENU_POS, GEN_MENU_WIDTH, GEN_MENU_HEIGHT, DISPLAY_WHITE);
    draw_credit_title();
    gfx_set_cursor(GEN_MENU_POS);
    gfx_set_text_background_color(HOME_MENU_BG_COLOR, DISPLAY_WHITE);
    gfx_puts("nsec 2019 badge team:");
    menu_init(&g_menu, GEN_MENU_POS_X, GEN_MENU_POS_Y + 8, GEN_MENU_WIDTH,
              GEN_MENU_HEIGHT - 8, ARRAY_SIZE(members_items), members_items,
              HOME_MENU_BG_COLOR, DISPLAY_WHITE);
    gfx_update();
}

static void draw_battery_title(void)
{
    draw_title("BATTERY", 25, 5, DISPLAY_BLUE, DISPLAY_WHITE);
}

void show_battery_status(void) {
    _state = SETTING_STATE_BATTERY;
    draw_battery_title();
    start_battery_status_timer();
}

static void turn_off_screen(uint8_t item) {
    display_set_brightness(0);
    _state = SETTING_STATE_SCREEN_OFF;
}

static bool setting_handle_buttons(button_t button, menu_t *menu)
{
    bool quit = false;

    if (button == BUTTON_BACK) {
        switch (_state) {
            case SETTING_STATE_MENU:
                _state = SETTING_STATE_CLOSED;
                quit = true;
                break;

            case SETTING_STATE_CREDIT_DETAILS:
                show_credit(4);
                break;

            case SETTING_STATE_BATTERY:
                stop_battery_status_timer();
                // Battery status was moved to the main menu
                // this is a hack to make it work with minimal
                // code change
                _state = SETTING_STATE_CLOSED;
                show_main_menu();
                break;

            case SETTING_STATE_SCREEN_OFF:
                display_set_brightness(get_stored_display_brightness());
                // no break
            case SETTING_STATE_CREDIT:
                nsec_setting_show();
                break;

            default:
                break;
        }
    } else {
        menu_button_handler(menu, button);
    }

    return quit;
}

void nsec_setting_show(void)
{
    menu_init(&g_menu, GEN_MENU_POS, GEN_MENU_WIDTH, GEN_MENU_HEIGHT,
              ARRAY_SIZE(settings_items), settings_items, HOME_MENU_BG_COLOR,
              DISPLAY_WHITE);

    redraw_settings_menu(&g_menu);

    _state = SETTING_STATE_MENU;

    while (true) {
        button_t btn;
        BaseType_t ret = xQueueReceive(button_event_queue, &btn, portMAX_DELAY);
        APP_ERROR_CHECK_BOOL(ret == pdTRUE);

        bool quit = setting_handle_buttons(btn, &g_menu);

        if (quit) {
            break;
        }
    }
}
