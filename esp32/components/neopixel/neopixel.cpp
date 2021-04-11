#include "neopixel.h"

#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>
#include <stdio.h>

#include "FX.h"
#include "FastLED.h"
#include "save.h"

#define NUM_LEDS 15
#define DATA_PIN_1 33
#define LED_TYPE WS2811
#define COLOR_ORDER RGB

WS2812FX NeoPixel::_ws2812fx;
TaskHandle_t NeoPixel::_displayTaskHandle;
bool NeoPixel::is_on;

void NeoPixel::init()
{
    FastLED.addLeds<LED_TYPE, DATA_PIN_1>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(3, 1000);
    NeoPixel::_ws2812fx.init(NUM_LEDS, leds, false);

    if (Save::save_data.neopixel_is_on) {
        NeoPixel::start();
    }
}

void NeoPixel::start() {
        setColor(Save::save_data.neopixel_color);
        setBrightness(Save::save_data.neopixel_brightness);
        setPublicMode(Save::save_data.neopixel_mode);
}
void NeoPixel::stop()
{
    if (NeoPixel::_displayTaskHandle) {
        NeoPixel::is_on = false;
    }
    NeoPixel::_ws2812fx.setBrightness(0);
    FastLED.setBrightness(0);
    Save::save_data.neopixel_is_on = false;
}

void NeoPixel::setColor(int color)
{
    if (color <= 0xffffff && color >= 0) {
        int color_corr = ((color >> 8) & 0x00ff00) | ((color << 8) & 0xff0000) |
                         (color & 0x0000ff);
        NeoPixel::_ws2812fx.setColor(0, color_corr);
        _color = color;
        Save::save_data.neopixel_color = color;
    }
}

void NeoPixel::setBrightness(uint8_t brightness)
{
    if (brightness <= 0xff && brightness >= 0) {
        FastLED.setBrightness(brightness);
        NeoPixel::_ws2812fx.setBrightness(brightness);
        _brightness = brightness;
        Save::save_data.neopixel_brightness = brightness;
    }
}

void NeoPixel::setPublicMode(uint8_t mode)
{
    NeoPixel::setMode(_unlocked_mode[mode % 10]);
    _public_mode = mode;
    Save::save_data.neopixel_mode = mode;
}

void NeoPixel::setMode(uint8_t mode)
{
    if (NeoPixel::_displayTaskHandle) {
        NeoPixel::is_on = false;
    }
    NeoPixel::_ws2812fx.setMode(0, mode);
    NeoPixel::is_on = true;
    xTaskCreate(&(NeoPixel::displayPatterns), "display_patterns", 1024, NULL, 5,
                &NeoPixel::_displayTaskHandle);

    _mode = mode;
}

uint8_t NeoPixel::getBrightNess()
{
    return _brightness;
}

uint16_t NeoPixel::getMode()
{
    return _mode;
}

uint16_t NeoPixel::getPublicMode()
{
    return _public_mode;
}

int NeoPixel::getColor()
{
    return _color;
}

void neopixel_set_brightness(uint8_t brightness)
{
    NeoPixel::getInstance().setBrightness(brightness);
}
void neopixel_set_mode(uint16_t mode)
{
    NeoPixel::getInstance().setMode(mode);
}
void neopixel_set_color(int color)
{
    NeoPixel::getInstance().setColor(color);
}
