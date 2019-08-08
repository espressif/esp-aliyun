/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
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
#include "esp_log.h"

#include "infra_compat.h"
#include "wifi_provision_api.h"

#include "app_entry.h"
#include "factory_handle.h"
#include "lightbulb.h"
#include "wrappers_extra.h"

static const char* TAG = "app main";

static void linkkit_event_monitor(int event)
{
    switch (event) {
        case IOTX_AWSS_START: // AWSS start without enbale, just supports device discover
            // operate led to indicate user
            ESP_LOGI(TAG, "IOTX_AWSS_START");
            break;
        case IOTX_AWSS_ENABLE: // AWSS enable, AWSS doesn't parse awss packet until AWSS is enabled.
            ESP_LOGI(TAG, "IOTX_AWSS_ENABLE");
            // operate led to indicate user
            break;
        case IOTX_AWSS_LOCK_CHAN: // AWSS lock channel(Got AWSS sync packet)
            ESP_LOGI(TAG, "IOTX_AWSS_LOCK_CHAN");
            // operate led to indicate user
            break;
        case IOTX_AWSS_PASSWD_ERR: // AWSS decrypt passwd error
            ESP_LOGE(TAG, "IOTX_AWSS_PASSWD_ERR");
            // operate led to indicate user
            break;
        case IOTX_AWSS_GOT_SSID_PASSWD:
            ESP_LOGI(TAG, "IOTX_AWSS_GOT_SSID_PASSWD");
            // operate led to indicate user
            break;
        case IOTX_AWSS_CONNECT_ADHA: // AWSS try to connnect adha (device
                                     // discover, router solution)
            ESP_LOGI(TAG, "IOTX_AWSS_CONNECT_ADHA");
            // operate led to indicate user
            break;
        case IOTX_AWSS_CONNECT_ADHA_FAIL: // AWSS fails to connect adha
            ESP_LOGE(TAG, "IOTX_AWSS_CONNECT_ADHA_FAIL");
            // operate led to indicate user
            break;
        case IOTX_AWSS_CONNECT_AHA: // AWSS try to connect aha (AP solution)
            ESP_LOGI(TAG, "IOTX_AWSS_CONNECT_AHA");
            // operate led to indicate user
            break;
        case IOTX_AWSS_CONNECT_AHA_FAIL: // AWSS fails to connect aha
            ESP_LOGE(TAG, "IOTX_AWSS_CONNECT_AHA_FAIL");
            // operate led to indicate user
            break;
        case IOTX_AWSS_SETUP_NOTIFY: // AWSS sends out device setup information
                                     // (AP and router solution)
            ESP_LOGI(TAG, "IOTX_AWSS_SETUP_NOTIFY");
            // operate led to indicate user
            break;
        case IOTX_AWSS_CONNECT_ROUTER: // AWSS try to connect destination router
            ESP_LOGI(TAG, "IOTX_AWSS_CONNECT_ROUTER");
            // operate led to indicate user
            break;
        case IOTX_AWSS_CONNECT_ROUTER_FAIL: // AWSS fails to connect destination
                                            // router.
            ESP_LOGE(TAG, "IOTX_AWSS_CONNECT_ROUTER_FAIL");
            // operate led to indicate user
            break;
        case IOTX_AWSS_GOT_IP: // AWSS connects destination successfully and got
                               // ip address
            ESP_LOGI(TAG, "IOTX_AWSS_GOT_IP");
            // operate led to indicate user
            break;
        case IOTX_AWSS_SUC_NOTIFY: // AWSS sends out success notify (AWSS
                                   // sucess)
            ESP_LOGI(TAG, "IOTX_AWSS_SUC_NOTIFY");
            // operate led to indicate user
            break;
        case IOTX_AWSS_BIND_NOTIFY: // AWSS sends out bind notify information to
                                    // support bind between user and device
            ESP_LOGI(TAG, "IOTX_AWSS_BIND_NOTIFY");
            awss_stop();
            // operate led to indicate user
            break;
        case IOTX_AWSS_ENABLE_TIMEOUT: // AWSS enable timeout
                                       // user needs to enable awss again to support get ssid & passwd of router
            ESP_LOGW(TAG, "IOTX_AWSS_ENALBE_TIMEOUT");
            // operate led to indicate user
            break;
        case IOTX_CONN_CLOUD: // Device try to connect cloud
            ESP_LOGI(TAG, "IOTX_CONN_CLOUD");
            // operate led to indicate user
            break;
        case IOTX_CONN_CLOUD_FAIL: // Device fails to connect cloud, refer to
                                   // net_sockets.h for error code
            ESP_LOGE(TAG, "IOTX_CONN_CLOUD_FAIL");
            // operate led to indicate user
            break;
        case IOTX_CONN_CLOUD_SUC: // Device connects cloud successfully
            ESP_LOGI(TAG, "IOTX_CONN_CLOUD_SUC");
            // operate led to indicate user
            break;
        case IOTX_RESET: // Linkkit reset success (just got reset response from
                         // cloud without any other operation)
            ESP_LOGI(TAG, "IOTX_RESET");
            // operate led to indicate user
            break;
        default:
            break;
    }
}

static void smart_light_example(void* parameter)
{
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());

    HAL_Kv_Init();
    HAL_Wifi_Init();
    factory_init();

    led_light_start();

    IOT_SetLogLevel(IOT_LOG_INFO);
    if (HAL_Wifi_Is_Network_Configured()) {
        HAL_Wifi_Sta_Connect(NULL, 0, NULL, 0, portMAX_DELAY);
    } else {
        // awss callback
        iotx_event_regist_cb(linkkit_event_monitor);
        
        // make sure user touches device belong to themselves
        awss_config_press();

        // awss entry
        awss_start();
        awss_check_reset();
    }
    
    while(1) {
        ESP_LOGI(TAG, "Network is Ready!");
        HAL_Wifi_Got_IP(portMAX_DELAY);

        app_main_paras_t paras;
        char* argv[] = {"main", "loop"};
        paras.argc = 2;
        paras.argv = argv;
        ESP_LOGI(TAG, "entry linkkit main...");
        linkkit_main((void *)&paras);
    }
}

void app_main()
{
    xTaskCreate(smart_light_example, "smart_light_example", 10240, NULL, 5, NULL);
}
