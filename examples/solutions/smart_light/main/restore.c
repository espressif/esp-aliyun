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

#include "dm_wrapper.h"

#define FACTORY_MIN_TIMEOUT        (6 * 1000)
#define FACTORY_TOTAL_COUNT        (5)
#define RESTORE_FACTORY_KEY        "factory_key"
#define RESTORE_WIFI_CONFIG_KEY    "ap_config"

static const char *TAG = "factory";

#define LINKKIT_RESTART_TIMEOUT_MS      (5000)
#define LINKKIT_STORE_RESTART_COUNT_KEY "restart_count"

static void restart_count_erase_timercb(void *timer)
{
    if (!xTimerStop(timer, portMAX_DELAY)) {
        ESP_LOGE(TAG,"xTimerStop timer: %p", timer);
    }

    if (!xTimerDelete(timer, portMAX_DELAY)) {
        ESP_LOGE(TAG,"xTimerDelete timer: %p", timer);
    }

    HAL_Kv_Del(LINKKIT_STORE_RESTART_COUNT_KEY);
    ESP_LOGW(TAG,"Erase restart count");
}

int restart_count_get(void)
{
    esp_err_t ret             = ESP_OK;
    TimerHandle_t timer       = NULL;
    uint32_t restart_count    = 0;
    int count_len        = sizeof(uint32_t);

    HAL_Kv_Get(LINKKIT_STORE_RESTART_COUNT_KEY, &restart_count, &count_len);

    /**< If the device restarts within the instruction time,
         the event_mdoe value will be incremented by one */
    restart_count++;
    ret = HAL_Kv_Set(LINKKIT_STORE_RESTART_COUNT_KEY, &restart_count, sizeof(uint32_t),0);
    if(ret != ESP_OK){
        ESP_LOGE(TAG,"Save the number of restarts within the set time");
    }

    timer = xTimerCreate("restart_count_erase", LINKKIT_RESTART_TIMEOUT_MS / portTICK_RATE_MS,
                         false, NULL, restart_count_erase_timercb);
    if(timer == NULL){
        ESP_LOGE(TAG,"xTaskCreate ERROR, timer: %p", timer);
    }

    xTimerStart(timer, 0);

    return restart_count;
}

esp_err_t erase_system_count(void)
{
    return HAL_Kv_Del(LINKKIT_STORE_RESTART_COUNT_KEY);
}