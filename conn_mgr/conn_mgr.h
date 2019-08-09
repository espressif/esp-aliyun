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

/**
 * Unregister System Event handler for External Provisioning.
 *
 * If a system handler is registered for external provisioning using HAL_Wifi_Register_Prov_Handler(),
 * the same can be unregistered using this call. 
 */
void conn_mgr_register_wifi_event(system_event_cb_t cb);

/**
 * @brief Restore the information of an AP
 * 
 * @return SUCCESS_RETURN on success
 * @return other on error
 */
esp_err_t conn_mgr_reset_wifi_config(void);

/**
 * @brief Initialize the information of wifi module
 * 
 * @return SUCCESS_RETURN on success
 * @return other on error
 */
esp_err_t conn_mgr_init(void);

/**
 * @brief Start the connect of wifi module to the AP.
 * 
 * This starts the wifi config and also connect it
 * 
 * @return SUCCESS_RETURN on success
 * @return other on error
 */
esp_err_t conn_mgr_start(void);

/**
 * @brief Initialize the information of key store
 *
 * @return SUCCESS_RETURN on success
 * @return other on error
 */
int HAL_Kv_Init(void);

#ifdef __cplusplus
} // extern "C"
#endif
