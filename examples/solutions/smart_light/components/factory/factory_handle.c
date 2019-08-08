/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
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
#include <string.h>
#include <esp_log.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#include "dm_wrapper.h"
#include "wrappers_extra.h"

#define FACTORY_MIN_TIMEOUT        (6 * 1000)
#define FACTORY_TOTAL_COUNT        (5)

#define LINKKIT_STORE_FACTORY_KEY "factory_key"

static const char *TAG = "factory";

static esp_err_t factory_handle(void)
{
    esp_err_t ret = ESP_OK;
    int reboot_num = 0;

    /**< If the device restarts within the instruction time, the event_mdoe value will be incremented by one */
    int length = sizeof(int);
    ret = HAL_Kv_Get(LINKKIT_STORE_FACTORY_KEY, &reboot_num, &length);

    reboot_num ++;

    ret = HAL_Kv_Set(LINKKIT_STORE_FACTORY_KEY, &reboot_num, sizeof(int), 0);
    if (reboot_num >= FACTORY_TOTAL_COUNT) {
        ESP_LOGE(TAG, "Handle Factory");
        HAL_Kv_Del(LINKKIT_STORE_FACTORY_KEY);
        HAL_Wifi_Del_Network();
    } else {
        ESP_LOGE(TAG, "Don't Factory");
    }

    return ESP_OK;
}

static void factory_clear(void *timer)
{
    if (!xTimerStop(timer, 0)) {
        ESP_LOGE(TAG, "xTimerStop timer %p", timer);
    }

    if (!xTimerDelete(timer, 0)) {
        ESP_LOGE(TAG, "xTimerDelete timer %p", timer);
    }

    /* erase reboot number record*/
    HAL_Kv_Del(LINKKIT_STORE_FACTORY_KEY);

    ESP_LOGI(TAG, "Timeout, must be clear");
}

int factory_init(void)
{
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED) {
        HAL_Kv_Del(LINKKIT_STORE_FACTORY_KEY);
        return ESP_OK;
    }

    TimerHandle_t timer = NULL;
    esp_err_t ret      = ESP_OK;

    timer = xTimerCreate("factory_clear", FACTORY_MIN_TIMEOUT / portTICK_RATE_MS,
                         false, NULL, factory_clear);

    xTimerStart(timer, portMAX_DELAY);

    ret = factory_handle();
    ESP_LOGD(TAG, "factory_handle, ret: %d", ret);

    return ESP_OK;
}