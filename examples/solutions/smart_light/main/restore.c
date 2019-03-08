/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS products only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/* Reset to Factory
*/
#include <string.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#include "platform_hal.h"

#define FACTORY_MIN_TIMEOUT        (6 * 1000)
#define FACTORY_TOTAL_COUNT        (5)
#define RESTORE_FACTORY_KEY        "factory_key"
#define RESTORE_WIFI_CONFIG_KEY    "wifi_config"

static const char *TAG = "factory";

ssize_t esp_info_save(const char *key, const void *value, size_t length);
int esp_info_erase(const char *key);

static esp_err_t restore_factory_handle()
{
    esp_err_t ret = ESP_OK;
    int reboot_num = 0;

    /**< If the device restarts within the instruction time, the event_mdoe value will be incremented by one */
    ret = esp_info_load(RESTORE_FACTORY_KEY, &reboot_num, sizeof(int));

    reboot_num ++;
    ret = esp_info_save(RESTORE_FACTORY_KEY, &reboot_num, sizeof(int));

    if (reboot_num >= FACTORY_TOTAL_COUNT) {
        ESP_LOGI(TAG, "Handle Factory %d", reboot_num);
        esp_info_erase(RESTORE_FACTORY_KEY);
        esp_info_erase(RESTORE_WIFI_CONFIG_KEY);
        esp_restart();
    } else {
        ESP_LOGI(TAG, "Don't Factory");
    }

    return ret;
}

static void restore_factory_clear(void *timer)
{
    if (!xTimerStop(timer, 0)) {
        ESP_LOGE(TAG, "xTimerStop timer %p", timer);
    }

    if (!xTimerDelete(timer, 0)) {
        ESP_LOGE(TAG, "xTimerDelete timer %p", timer);
    }

    /* erase reboot number record*/
    esp_info_erase(RESTORE_FACTORY_KEY);

    ESP_LOGI(TAG, "Timeout, must be clear");
}

int restore_factory_init(void)
{
    TimerHandle_t timer = NULL;
    esp_err_t ret      = ESP_OK;

    timer = xTimerCreate("restore_factory", FACTORY_MIN_TIMEOUT / portTICK_RATE_MS,
                         false, NULL, restore_factory_clear);

    xTimerStart(timer, portMAX_DELAY);

    ret = restore_factory_handle();

    return ret;
}

