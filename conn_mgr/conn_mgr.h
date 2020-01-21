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

#pragma once

#include "esp_err.h"
#include "esp_event_loop.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HOTSPOT_AP "aha"
#define ROUTER_AP "adha"

/**
 * @brief register wifi event handler
 *
 * @param cb wifi event handler
 *
 * @return none
 */
void conn_mgr_register_wifi_event(system_event_cb_t cb);

/**
 * @brief reset the stored router info, include ssid & password
 * 
 * @return
 *     - ESP_OK : OK
 *     - others : fail
 */
esp_err_t conn_mgr_reset_wifi_config(void);

/**
 * @brief connect wifi with configure information 
 * 
 * This will initiate connection to the given Wi-Fi configure information
 * 
 * @param ssid Pointer to the target network SSID string
 * @param ssid_len Length of the above SSID
 * @param password Pointer to the targer network Password string. Can be NULL for open networks.
 * @param password_len Length of the password
 * 
 * @return
 *     - ESP_OK : OK
 *     - others : fail
 */
esp_err_t conn_mgr_set_wifi_config_ext(const uint8_t *ssid, size_t ssid_len, const uint8_t *password, size_t password_len);

/**
 * @brief get wifi configure information
 * 
 * @return
 *     - ESP_OK : OK
 *     - others : fail
 */
esp_err_t conn_mgr_get_wifi_config(wifi_config_t *wifi_cfg);

/**
 * @brief init the connection management module
 * 
 * @return
 *     - ESP_OK : OK
 *     - others : fail
 */
esp_err_t conn_mgr_init(void);

/**
 * @brief start the connection management module
 * 
 * If the device is configured, the device will connect to the router which is configured.
 * If the device is not configured, the device will start awss service.
 * 
 * @return
 *     - ESP_OK : OK
 *     - others : fail
 */
esp_err_t conn_mgr_start(void);

/**
 * @brief stop the connection management module
 * 
 * If the device is configured, the device keep connect to the router which is configured.
 * If the device is not configured, the device will stop awss service.
 * 
 * @return
 *     - ESP_OK : OK
 *     - others : fail
 */
esp_err_t conn_mgr_stop(void);

#ifdef __cplusplus
}
#endif
