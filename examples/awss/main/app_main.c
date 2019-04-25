/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP32 only, in which case,
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
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi_provision_api.h"

#include "infra_compat.h"
#include "iot_import_awss.h"
#include "smartconfig_wrapper.h"

// for demo only
#define PRODUCT_KEY      "a1X2bEnP82z"
#define PRODUCT_SECRET   "7jluWm1zql7bt8qK"
#define DEVICE_NAME      "test_06"
#define DEVICE_SECRET    "wQ1xOzFH3kLdjCTLfi8Xbw4otRz0lHoq"

static const char* TAG = "app main";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGW(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            esp_wifi_connect();
            break;

        default:
            break;
    }

    return ESP_OK;
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    // esp_init_wifi_event_group();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main()
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    initialise_wifi();

    awss_set_config_press(1);

    // awss entry
    awss_start();
    // HAL_Awss_Open_Monitor(set_iotx_info);
    // vTaskDelay(1000);
    // HAL_Awss_Close_Monitor();
    // HAL_Awss_Connect_Ap(3000000, "Airport_FAST", "Airport_FAST", 0, 0, NULL, 0);
    ESP_LOGI(TAG, "Network is Ready!");
