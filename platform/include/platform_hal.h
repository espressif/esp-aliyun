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
#ifndef PLATFORM_HAL_H_
#define PLATFORM_HAL_H_

#include <stdint.h>
#include "esp_event_loop.h"
/*
* @brief init wifi event group
* This function should be called when init wifi if using awss
*/
void esp_init_wifi_event_group();

/**
 * @brief Wait for device got IP address
 *
 * @note should call `esp_init_wifi_event_group` firstly
 *
 * @param block_time_tick: ticks for wait
 *                          = 0: will block
 *                         != 0:  will wait for block_time_tick ticks
 */
uint32_t HAL_Wait_Net_Ready(uint32_t block_time_tick);

/*
* @brief provide wifi event callback for user
*
* awss already has wifi event callback, and will control all wifi state
* here will provide wifi event for user to know what wifi event happened
* please do not call system wifi interface in cb
*
* @param cb: callback for user, will call it when system wifi event happened
*/
void set_user_wifi_event_cb(system_event_cb_t cb);

/*
* @brief nvs load key-value
*
* get saved config from the flash
* 
*/
ssize_t esp_info_load(const char *key, void *value, size_t length);

// version of iotkit-embedded
const char* HAL_GetIEVerison(void);

// version of esp-aliyun
const char* HAL_GetEAVerison(void);

#endif // !PLATFORM_HAL_H_
