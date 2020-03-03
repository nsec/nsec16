//  Copyright (c) 2017
//  Eric Tremblay <habscup@gmail.com>
//
//  License: MIT (see LICENSE for details)

#include "home_menu.h"
#include "FreeRTOS.h"
#include "app_screensaver.h"
#include "application.h"
#include "drivers/battery_manager.h"
#include "drivers/buttons.h"
#include "drivers/controls.h"
#include "drivers/display.h"
#include "gfx_effect.h"
#include "gui.h"
#include "main_menu.h"
#include "menu.h"
#include "nsec_settings.h"
#include "persistency.h"
#include "queue.h"
#include "status_bar.h"
#include "timer.h"
#include "ui_page.h"

#include <string.h>

#include "images/neurosoft_logo_bitmap.h"
#include "images/neurosoft_logo_a_1_bitmap.h"
#include "images/neurosoft_logo_a_2_bitmap.h"
#include "images/neurosoft_logo_a_3_bitmap.h"
#include "images/neurosoft_logo_a_4_bitmap.h"
#include "images/neurosoft_logo_a_5_bitmap.h"
#include "images/neurosoft_logo_a_6_bitmap.h"
#include "images/neurosoft_logo_a_7_bitmap.h"
#include "images/neurosoft_logo_a_8_bitmap.h"
#include "images/neurosoft_logo_a_9_bitmap.h"
#include "images/neurosoft_logo_a_10_bitmap.h"
#include "images/neurosoft_logo_a_11_bitmap.h"
#include "images/neurosoft_logo_a_12_bitmap.h"
#include "images/neurosoft_logo_a_13_bitmap.h"
#include "images/neurosoft_logo_a_14_bitmap.h"
#include "images/neurosoft_logo_a_15_bitmap.h"
#include "images/settings_off_bitmap.h"
#include "images/settings_on_bitmap.h"

static uint16_t gfx_width;
static uint16_t gfx_height;

#ifdef NSEC_FLAVOR_CTF
static uint32_t g_last_ms;
#endif

static enum {
    HOME_STATE_MAIN_MENU_SELECTED,
    HOME_STATE_SETTINGS_SELECTED
} _state = HOME_STATE_MAIN_MENU_SELECTED;

static void draw_burger_menu_icon(int16_t x, int16_t y, uint16_t color) {
    gfx_fill_rect(x, y, 3, 3, color);
    gfx_fill_rect(x, y + 6, 3, 3, color);
    gfx_fill_rect(x, y + 12, 3, 3, color);

    gfx_fill_rect(x + 6, y, 12, 3, color);
    gfx_fill_rect(x + 6, y + 6, 12, 3, color);
    gfx_fill_rect(x + 6, y + 12, 12, 3, color);
}

static void draw_cursor(void) {
    if (_state == HOME_STATE_MAIN_MENU_SELECTED) {
        gfx_fill_triangle(HOME_MENU_POS_X + 1, SETTINGS_MENU_CUR_Y,
                          HOME_MENU_POS_X + 7, SETTINGS_MENU_CUR_Y + 5,
                          HOME_MENU_POS_X + 1, SETTINGS_MENU_CUR_Y + 10,
                          HOME_MENU_BG_COLOR);

        gfx_fill_triangle(HOME_MENU_POS_X + 1, BURGER_MENU_CUR_Y,
                          HOME_MENU_POS_X + 7, BURGER_MENU_CUR_Y + 5,
                          HOME_MENU_POS_X + 1, BURGER_MENU_CUR_Y + 10,
                          DISPLAY_RED);
    } else {
        gfx_fill_triangle(HOME_MENU_POS_X + 1, BURGER_MENU_CUR_Y,
                          HOME_MENU_POS_X + 7, BURGER_MENU_CUR_Y + 5,
                          HOME_MENU_POS_X + 1, BURGER_MENU_CUR_Y + 10,
                          HOME_MENU_BG_COLOR);

        gfx_fill_triangle(HOME_MENU_POS_X + 1, SETTINGS_MENU_CUR_Y,
                          HOME_MENU_POS_X + 7, SETTINGS_MENU_CUR_Y + 5,
                          HOME_MENU_POS_X + 1, SETTINGS_MENU_CUR_Y + 10,
                          DISPLAY_RED);
    }
}

void draw_title(const char *text, uint8_t pos_x, uint8_t pos_y,
                uint16_t text_color, uint16_t bg_color)
{
    gfx_fill_rect(0, 0, gfx_width - HOME_MENU_WIDTH, GEN_MENU_POS_Y, bg_color);

    gfx_set_cursor(pos_x, pos_y);
    gfx_set_text_size(2);
    gfx_set_text_background_color(text_color, bg_color);
    gfx_puts(text);
    gfx_set_text_size(1);
}

void draw_settings_title(void)
{
    draw_title("SETTINGS", SETTINGS_MENU_TITLE_X, 5, DISPLAY_BLUE,
               DISPLAY_WHITE);
}

void draw_main_menu_title(void)
{
    draw_title("MENU", SETTINGS_MENU_TITLE_X, 5, DISPLAY_BLUE, DISPLAY_WHITE);
}

static void draw_home_menu_bar(void)
{
    gfx_fill_rect(HOME_MENU_POS, HOME_MENU_WIDTH, HOME_MENU_HEIGHT,
                  HOME_MENU_BG_COLOR);

    draw_burger_menu_icon(BURGER_MENU_POS, DISPLAY_WHITE);

    gfx_draw_16bit_bitmap(SETTINGS_MENU_POS, &settings_off_bitmap,
                          HOME_MENU_BG_COLOR);
}

void redraw_home_menu_burger_selected(void)
{
    gfx_fill_rect(HOME_MENU_POS, HOME_MENU_WIDTH, HOME_MENU_HEIGHT / 2,
                  DISPLAY_WHITE);

    gfx_fill_rect(HOME_MENU_POS_X, HOME_MENU_POS_Y + (HOME_MENU_HEIGHT / 2),
                  HOME_MENU_WIDTH, HOME_MENU_HEIGHT / 2, HOME_MENU_BG_COLOR);

    gfx_draw_16bit_bitmap(SETTINGS_MENU_POS, &settings_off_bitmap,
                          HOME_MENU_BG_COLOR);

    draw_burger_menu_icon(BURGER_MENU_POS, HOME_MENU_BG_COLOR);
}

#ifdef NSEC_FLAVOR_CONF
#include "images/external/conf/nsec_logo_color_bitmap.h"

static void home_page_redraw(void)
{
    draw_home_menu_bar();
    draw_cursor();

    gfx_fill_rect(0, 0, gfx_width - HOME_MENU_WIDTH, gfx_height, DISPLAY_BLACK);

    display_draw_16bit_ext_bitmap(NSEC_LOGO_POS, &nsec_logo_color_bitmap,
                                  DISPLAY_BLACK);

    gfx_set_cursor(CONF_STR_POS);
    gfx_set_text_background_color(DISPLAY_WHITE, DISPLAY_BLACK);
    gfx_set_text_size(1);
    gfx_puts("Conference");
}
#else
static void draw_home_menu_logo_animation(void)
{
    static uint8_t frame = 0;

    switch (frame) {
    case 0:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_1_bitmap, 0);
        break;

    case 3:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_2_bitmap, 0);
        break;

    case 6:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_3_bitmap, 0);
        break;

    case 9:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_4_bitmap, 0);
        break;

    case 12:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_5_bitmap, 0);
        break;

    case 15:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_6_bitmap, 0);
        break;

    case 18:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_7_bitmap, 0);
        break;

    case 21:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_8_bitmap, 0);
        break;

    case 24:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_9_bitmap, 0);
        break;

    case 27:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_10_bitmap, 0);
        break;

    case 30:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_11_bitmap, 0);
        break;

    case 33:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_12_bitmap, 0);
        break;

    case 36:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_13_bitmap, 0);
        break;

    case 39:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_14_bitmap, 0);
        break;

    case 42:
        gfx_draw_16bit_bitmap(NEUROSOFT_ANIMATION_POS,
                              &neurosoft_logo_a_15_bitmap, 0);
        break;
    }

    frame = frame >= 45 ? 0 : frame + 1;
}

static void home_page_redraw(void)
{
    draw_home_menu_bar();
    draw_cursor();

    gfx_fill_rect(0, 0, gfx_width - HOME_MENU_WIDTH, gfx_height, DISPLAY_BLACK);

    gfx_draw_16bit_bitmap(NEUROSOFT_LOGO_POS, &neurosoft_logo_bitmap,
                          DISPLAY_BLACK);
}
#endif

static void open_burger_menu(void) {
    gfx_fill_rect(0, 0, gfx_width - HOME_MENU_WIDTH, gfx_height, DISPLAY_WHITE);

    gfx_fill_rect(HOME_MENU_POS, HOME_MENU_WIDTH, HOME_MENU_HEIGHT / 2,
                  DISPLAY_WHITE);

    draw_burger_menu_icon(BURGER_MENU_POS, HOME_MENU_BG_COLOR);

    draw_main_menu_title();

    show_ui_page(&main_menu_page, NULL);
}

static void open_settings_menu(void) {
    gfx_fill_rect(0, 0, gfx_width - HOME_MENU_WIDTH, gfx_height, DISPLAY_WHITE);

    gfx_fill_rect(HOME_MENU_POS_X, HOME_MENU_POS_Y + (HOME_MENU_HEIGHT / 2),
                  HOME_MENU_WIDTH, HOME_MENU_HEIGHT / 2, DISPLAY_WHITE);

    gfx_draw_16bit_bitmap(SETTINGS_MENU_POS, &settings_on_bitmap,
                          DISPLAY_WHITE);

    show_ui_page(&settings_menu_page, NULL);
}

static bool home_page_handle_button(button_t button)
{
    /* Reset the screensaver timeout on each button event */
    // screensaver_reset();

    switch (button) {
    case BUTTON_BACK:
        break;

    case BUTTON_UP:
    case BUTTON_DOWN:
        if (_state == HOME_STATE_MAIN_MENU_SELECTED) {
            _state = HOME_STATE_SETTINGS_SELECTED;
            draw_cursor();
        } else if (_state == HOME_STATE_SETTINGS_SELECTED) {
            _state = HOME_STATE_MAIN_MENU_SELECTED;
            draw_cursor();
        }
        break;

    case BUTTON_ENTER:
        if (_state == HOME_STATE_MAIN_MENU_SELECTED) {
            open_burger_menu();
        } else {
            open_settings_menu();
        }
        break;

    default:
        break;
    }

#ifdef NSEC_FLAVOR_CTF
    /* Animate the logo. */
    uint32_t this_ms = get_current_time_millis();
    if (this_ms - g_last_ms > 20) {
        draw_home_menu_logo_animation();
        g_last_ms = this_ms;
    }
#endif

    /* Never quit. */
    return false;
}

static void home_page_init(void *init_data)
{
    gfx_width = gfx_get_screen_width();
    gfx_height = gfx_get_screen_height();
#ifdef NSEC_FLAVOR_CTF
    g_last_ms = get_current_time_millis();
#endif
}

// To be removed.
bool is_at_home_menu(void)
{
    return false;
}

const ui_page home_page = {
    .init = home_page_init,
    .redraw = home_page_redraw,
    .handle_button = home_page_handle_button,
    .ticks_timeout = 25,
};
