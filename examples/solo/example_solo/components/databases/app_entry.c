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
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_system.h"
#include "transport_uart.h"
#include "transport_data.h"
#include "conn_mgr.h"
#include "dm_wrapper.h"
#include "wifi_provision_api.h"
#include "app_entry.h"

static const char *TAG = "app_entry";

#define AWSS_SC_NAME                 "active_awss"
#define AWSS_SOFTAP_NAME             "dev_ap"
#define AWSS_RESET_NAME              "reset"
#define AWSS_CONN_NAME               "netmgr connect"
#define AWSS_CONFIG_NAME             "linkkey"
#define AWSS_REBOOT_NAME             "reboot"
#define AWSS_KV_ERASE_DY_SECRET_NAME "kv_clear"
#define DYNAMIC_REG_KV_PREFIX       "DYNAMIC_REG_"
#define DYNAMIC_REG_KV_PREFIX_LEN   12

static bool s_conn_mgr_exist = false;

int app_check_config_pk(void)
{
    char PRODUCT_KEY[IOTX_PRODUCT_KEY_LEN + 1] = {0};

    int ret = HAL_GetProductKey(PRODUCT_KEY);

    if (!ret || !strlen(PRODUCT_KEY)) {
        ESP_LOGE(TAG, "Please first input four config");
        return 0;
    }

    return 1;
}

static void app_get_config_input_len(const char *param, uint32_t *len)
{
    uint32_t i = 0;
    for (i = 0; i < strlen(param); i ++) {
        if (param[i] == ' ' || param[i] == '\r' || param[i] == '\n') {
            break;
        }
    }
    *len = i;
}

void start_conn_mgr(void)
{
    s_conn_mgr_exist = true;
    conn_mgr_start();
    s_conn_mgr_exist = false;
    vTaskDelete(NULL);
}

void app_get_input_param(char *param, size_t param_len)
{
    if (!param) {
        ESP_LOGE(TAG, "Input error");
        return;
    }

    if (!strncmp(param, AWSS_REBOOT_NAME, strlen(AWSS_REBOOT_NAME))) {
        ESP_LOGI(TAG, "Reboot now");
        esp_restart();
    } else if (!strncmp(param, AWSS_KV_ERASE_DY_SECRET_NAME, strlen(AWSS_KV_ERASE_DY_SECRET_NAME))) {
        ESP_LOGI(TAG, "Clear DY DeviceSecrt KV");
        char kv_key[IOTX_DEVICE_NAME_LEN + DYNAMIC_REG_KV_PREFIX_LEN] = DYNAMIC_REG_KV_PREFIX;
        char DEVICE_NAME[IOTX_DEVICE_NAME_LEN + 1] = {0};
        HAL_GetDeviceName(DEVICE_NAME);
        memcpy(kv_key + strlen(kv_key), DEVICE_NAME, strlen(DEVICE_NAME));
        HAL_Kv_Del(kv_key);
        return;
    }

    if (strstr(param, AWSS_CONFIG_NAME)) {
        uint32_t len = 0;
        char buf[64 + 1] = {0};

        char *input = param + strlen(AWSS_CONFIG_NAME) + 1;
        app_get_config_input_len(input, &len);
        strncpy(buf, input, len);
        ESP_LOGI(TAG, "ProductKey: %s", buf);
        HAL_SetProductKey(buf);

        input += len + 1;
        app_get_config_input_len(input, &len);
        memset(buf, 0, 65);
        strncpy(buf, input, len);
        ESP_LOGI(TAG, "DeviceName: %s", buf);
        HAL_SetDeviceName(buf);

        input += len + 1;
        app_get_config_input_len(input, &len);
        memset(buf, 0, 65);
        strncpy(buf, input, len);
        ESP_LOGI(TAG, "DeviceSecret: %s", buf);
        HAL_SetDeviceSecret(buf);

        input += len + 1;
        app_get_config_input_len(input, &len);
        memset(buf, 0, 65);
        strncpy(buf, input, len);
        ESP_LOGI(TAG, "ProductSecret: %s", buf);
        HAL_SetProductSecret(buf);
        return;
    }

    if (!app_check_config_pk()) {
        return;
    }

    conn_mgr_stop();

    if (strstr(param, AWSS_SC_NAME)) {
        ESP_LOGI(TAG, "Set smartconfig and zero config");
        if (s_conn_mgr_exist) {
            ESP_LOGE(TAG, "In AWSS config can't set sc mode");
            return;
        }
        conn_mgr_set_sc_mode(CONN_SC_ZERO_MODE);
    } else if (strstr(param, AWSS_SOFTAP_NAME)) {
        ESP_LOGI(TAG, "Set softap config");
        if (s_conn_mgr_exist) {
            ESP_LOGE(TAG, "In AWSS config can't set sc mode");
            return;
        }
        conn_mgr_set_sc_mode(CONN_SOFTAP_MODE);
    } else if (strstr(param, AWSS_RESET_NAME)) {
        conn_mgr_reset_wifi_config();
        ESP_LOGI(TAG, "Reset and unbind device");
        awss_report_reset();
        vTaskDelay(2000 / portTICK_RATE_MS);
        esp_restart();
    } else if (strstr(param, AWSS_CONN_NAME)) {
        uint32_t len = 0;
        char buf[64 + 1] = {0};

        char *input = param + strlen(AWSS_CONN_NAME) + 1;
        app_get_config_input_len(input, &len);
        strncpy(buf, input, len);
        ESP_LOGI(TAG, "SSID: %s", buf);
        HAL_Kv_Set(STA_SSID_KEY, buf, 32, 0);

        input += len + 1;
        app_get_config_input_len(input, &len);
        memset(buf, 0 ,65);
        strncpy(buf, input, len);
        ESP_LOGI(TAG, "Password: %s", buf);
        HAL_Kv_Set(STA_PASSWORD_KEY, buf, 64, 0);
    } else {
        ESP_LOGE(TAG, "Can't recongize cmd");
        return;
    }

    xTaskCreate(start_conn_mgr, "conn_mgr", CONN_MGR_TASK_SIZE, NULL, 5, NULL);
}
