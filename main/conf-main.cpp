/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright 2024 Jérémie Galarneau <jeremie.galarneau@gmail.com>
 */

#include "buttons.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include <stdio.h>

#include <iostream>

#include <badge/badge.hpp>
#include <badge/globals.hpp>
#include <scheduling/task.hpp>

namespace
{
// Prevent the linker from eliminating the badge instance by forcing a
// reference to be accounted-for.
const auto &_badge_ref __attribute__((__used__)) = &nsec::g::the_badge;
} // namespace

class dummy_task : public nsec::scheduling::periodic_task<dummy_task>
{
    friend class periodic_task<dummy_task>;

  public:
    dummy_task() : nsec::scheduling::periodic_task<dummy_task>(1000)
    {
        start();
    }

    const char *name() const noexcept
    {
        return "dummy task";
    }

  private:
    void tick(nsec::scheduling::absolute_time_ms current_time_ms
              [[maybe_unused]])
    {
        std::cout << "Hello from dummy task" << std::endl;
    }
};

extern "C" void app_main(void)
{
    //const dummy_task the_task;

    buttons_init();
    xTaskCreate(buttons_task, "buttons_task", 4096, NULL, 10, NULL);

    while (true) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
