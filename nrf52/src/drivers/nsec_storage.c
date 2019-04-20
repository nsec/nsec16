/*
 * Copyright 2018 Eric Tremblay <habscup@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "boards.h"
#include "nordic_common.h"
#include "nrf.h"
#include <nrf_delay.h>

#include "nrf_fstorage_sd.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"

#include "app/gfx_effect.h"
#include "app/identity.h"
#include "app/nsec_led_settings.h"
#include "display.h"
#include "led_code_storage.h"
#include "nsec_storage.h"
#include "power.h"
#include "ws2812fx.h"

#define PAGE_START_MAGIC 0xDEADC0DE

extern bool is_at_home_menu;

bool is_init = false;

static void wait_for_flash_ready(nrf_fstorage_t const *p_fstorage) {
    /* While fstorage is busy*/
    while (nrf_fstorage_is_busy(p_fstorage)) {
        power_manage();
    }
}

/* code interface */

static bool is_new_memory_page(nrf_fstorage_t const *p_fstorage) {
    ret_code_t rc;
    uint32_t new_dev_memory;
    rc = nrf_fstorage_read(p_fstorage, p_fstorage->start_addr, &new_dev_memory,
                           4);
    APP_ERROR_CHECK(rc);
    wait_for_flash_ready(p_fstorage);

    return (new_dev_memory == 0xFFFFFFFF) ? true : false;
}

void nsec_storage_init(void) {

    if (is_init) {
        return;
    }

    is_init = true;
}
