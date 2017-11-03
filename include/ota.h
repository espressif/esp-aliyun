/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
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

#ifndef __SYSTEM_CMD_H__
#define __SYSTEM_CMD_H__

#include "esp_common.h"

/*IMPORTANT: the following configuration maybe need to be modified*/
/***********************************************************************************************************************/
#define LOCAL_OTA_SERVER_IP       "192.168.111.104"     // local OTA server ip
#define LOCAL_OTA_SERVER_PORT     3344                  // local OTA server port

#define OTA_TIMEOUT 120000  // timeout: 120000 ms, both for local OTA & remote OTA

#define OTA_BUF_LEN 1460    // remote OTA buffer

#define ERASE_FLASH_SIZE 400 * 1024     // remote OTA bin maxium size
/***********************************************************************************************************************/


/*Please Keep the following configuration if you have no very deep understanding of ESP8266-OTA */
#define SPI_FLASH_SEC_SIZE      4096

// for local OTA
#define USER_BIN1               0x00
#define USER_BIN2               0x01

#define UPGRADE_FLAG_IDLE       0x00
#define UPGRADE_FLAG_START      0x01
#define UPGRADE_FLAG_FINISH     0x02

#define UPGRADE_FW_BIN1         0x00
#define UPGRADE_FW_BIN2         0x01

void system_upgrade_init();
bool system_upgrade(uint8 *data, uint32 len);
void system_upgrade_deinit(void);

void upgrade_recycle(void);

void local_ota_proc(void *pvParameter);
void remote_ota_proc(void *pvParameter);

#endif
