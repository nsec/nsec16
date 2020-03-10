//  Copyright (c) 2018
//  Eric Tremblay <habscup@gmail.com>
//
//  License: MIT (see LICENSE for details)

#include "led_effects.h"
#include "boards.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_pwm.h"
#include <nrf_delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// 3 leds/bytes by pixel
#define NSEC_NEOPIXEL_NUM_BYTES (NEOPIXEL_COUNT * 3)

struct Nsec_pixels {
    uint8_t brightness;
    uint8_t rOffset;
    uint8_t gOffset;
    uint8_t bOffset;
    uint8_t pixels[NSEC_NEOPIXEL_NUM_BYTES];
};

static struct Nsec_pixels g_nsec_pixels;

uint32_t mapConnect[] = {PIN_NEOPIXEL, NRF_PWM_PIN_NOT_CONNECTED,
                         NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED};

static uint32_t mapDisconnect[] = {
    NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED,
    NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED};

static bool initialized = false;

void nsec_neoPixel_init(void)
{
    initialized = true;

    memset(&g_nsec_pixels, 0, sizeof(g_nsec_pixels));

    // Magic number coming from Adafruit library
    g_nsec_pixels.rOffset = (NEO_GRB >> 4) & 0b11;
    g_nsec_pixels.gOffset = (NEO_GRB >> 2) & 0b11;
    g_nsec_pixels.bOffset = NEO_GRB & 0b11;

    // Configure pin
    nrf_gpio_cfg_output(PIN_NEOPIXEL);
    nrf_gpio_pin_clear(PIN_NEOPIXEL);
}

void nsec_neoPixel_clear(void) {
    memset(g_nsec_pixels.pixels, 0, sizeof(g_nsec_pixels.pixels));
}

// Set the n pixel color
void nsec_neoPixel_set_pixel_color(uint16_t n, uint8_t r, uint8_t g,
                                   uint8_t b) {
    if (n < NEOPIXEL_COUNT) {
        if (g_nsec_pixels.brightness) {
            r = (r * g_nsec_pixels.brightness) >> 8;
            g = (g * g_nsec_pixels.brightness) >> 8;
            b = (b * g_nsec_pixels.brightness) >> 8;
        }

        uint8_t *p;
        p = &g_nsec_pixels.pixels[n * 3];
        p[g_nsec_pixels.rOffset] = r;
        p[g_nsec_pixels.gOffset] = g;
        p[g_nsec_pixels.bOffset] = b;
    }
}

void nsec_neoPixel_set_pixel_color_packed(uint16_t n, uint32_t c) {
    if (n < NEOPIXEL_COUNT) {
        uint8_t r = (uint8_t)(c >> 16);
        uint8_t g = (uint8_t)(c >> 8);
        uint8_t b = (uint8_t)c;

        nsec_neoPixel_set_pixel_color(n, r, g, b);
    }
}

uint32_t nsec_neoPixel_get_pixel_color(uint16_t n) {
    if (n >= NEOPIXEL_COUNT)
        return 0;

    uint8_t *pixel;
    pixel = &g_nsec_pixels.pixels[n * 3];
    if (g_nsec_pixels.brightness) {
        return (((uint32_t)(pixel[g_nsec_pixels.rOffset] << 8) /
                 g_nsec_pixels.brightness)
                << 16) |
               (((uint32_t)(pixel[g_nsec_pixels.gOffset] << 8) /
                 g_nsec_pixels.brightness)
                << 8) |
               ((uint32_t)(pixel[g_nsec_pixels.bOffset] << 8) /
                g_nsec_pixels.brightness);
    } else {
        return ((uint32_t)pixel[g_nsec_pixels.rOffset] << 16) |
               ((uint32_t)pixel[g_nsec_pixels.gOffset] << 8) |
               (uint32_t)pixel[g_nsec_pixels.bOffset];
    }
}

void nsec_neoPixel_set_brightness(uint8_t b) {
    uint8_t newBrightness = b + 1;
    if (newBrightness != g_nsec_pixels.brightness) {
        uint8_t oldBrighness = g_nsec_pixels.brightness - 1;
        uint16_t scale;

        if (oldBrighness == 0) {
            scale = 0;
        } else if (b == 255) {
            scale = 65535;
        } else {
            scale = (((uint16_t)newBrightness << 8) - 1) / oldBrighness;
        }

        for (size_t i = 0; i < ARRAY_SIZE(g_nsec_pixels.pixels); i++) {
            uint8_t pixel = g_nsec_pixels.pixels[i];
            g_nsec_pixels.pixels[i] = (pixel * scale) >> 8;
        }
        g_nsec_pixels.brightness = newBrightness;
    }
}

uint8_t nsec_neoPixel_get_brightness(void) {
    return g_nsec_pixels.brightness - 1;
}

static void show_with_PWM(void)
{
    // todo Implement the canshow
    uint16_t pixels_pattern[NSEC_NEOPIXEL_NUM_BYTES * 8 + 2];
    uint16_t pos = 0;

    for (size_t n = 0; n < ARRAY_SIZE(g_nsec_pixels.pixels); n++) {
        uint8_t pix = g_nsec_pixels.pixels[n];

        for (uint8_t mask = 0x80, i = 0; mask > 0; mask >>= 1, i++) {
            pixels_pattern[pos] = (pix & mask) ? MAGIC_T1H : MAGIC_T0H;
            pos++;
        }
    }
    // Zero padding to indicate the end of que sequence
    pixels_pattern[++pos] = 0 | (0x8000);
    pixels_pattern[++pos] = 0 | (0x8000);

    nrf_pwm_configure(NRF_PWM0, NRF_PWM_CLK_16MHz, NRF_PWM_MODE_UP, CTOPVAL);
    nrf_pwm_loop_set(NRF_PWM0, 0);
    nrf_pwm_decoder_set(NRF_PWM0, NRF_PWM_LOAD_COMMON, NRF_PWM_STEP_AUTO);

    // Configure the sequence
    nrf_pwm_seq_ptr_set(NRF_PWM0, 0, pixels_pattern);
    nrf_pwm_seq_cnt_set(NRF_PWM0, 0, ARRAY_SIZE(pixels_pattern));
    nrf_pwm_seq_refresh_set(NRF_PWM0, 0, 0);
    nrf_pwm_seq_end_delay_set(NRF_PWM0, 0, 0);
    nrf_pwm_pins_set(NRF_PWM0, mapConnect);

    // Enable the PWM
    nrf_pwm_enable(NRF_PWM0);

    // Start the sequence
    nrf_pwm_event_clear(NRF_PWM0, NRF_PWM_EVENT_SEQEND0);
    nrf_pwm_task_trigger(NRF_PWM0, NRF_PWM_TASK_SEQSTART0);

    while (!nrf_pwm_event_check(NRF_PWM0, NRF_PWM_EVENT_SEQEND0))
        ;

    nrf_pwm_event_clear(NRF_PWM0, NRF_PWM_EVENT_SEQEND0);

    // Disable the PWM
    nrf_pwm_disable(NRF_PWM0);

    nrf_pwm_pins_set(NRF_PWM0, mapDisconnect);
}

void nsec_neoPixel_show(void)
{
    ASSERT(initialized);

    show_with_PWM();

    nrf_delay_us(50);
}
