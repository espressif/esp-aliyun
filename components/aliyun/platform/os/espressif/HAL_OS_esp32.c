/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"

#include "iot_import.h"

#ifdef MQTT_ID2_AUTH
#include "tfs.h"
#endif /**< MQTT_ID2_AUTH*/

typedef xSemaphoreHandle osi_mutex_t;

char _product_key[PRODUCT_KEY_LEN + 1];
char _product_secret[PRODUCT_SECRET_LEN + 1];
char _device_name[DEVICE_NAME_LEN + 1];
char _device_secret[DEVICE_SECRET_LEN + 1];
#define UNUSED(expr) do { (void)(expr); } while (0)

void *HAL_MutexCreate(void)
{
    osi_mutex_t *p_mutex = NULL;
    p_mutex = (osi_mutex_t *)malloc(sizeof(osi_mutex_t));
    if(p_mutex == NULL)
        return NULL;

    *p_mutex = xSemaphoreCreateMutex();
    return p_mutex;
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
    vSemaphoreDelete(*((osi_mutex_t*)mutex));
    free(mutex);
}

void HAL_MutexLock(_IN_ void *mutex)
{
    xSemaphoreTake(*((osi_mutex_t*)mutex), portMAX_DELAY);
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
    xSemaphoreGive(*((osi_mutex_t*)mutex));
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    return malloc(size);
}

void HAL_Free(_IN_ void *ptr)
{
    free(ptr);
}

uint64_t HAL_UptimeMs(void)
{
    struct timeval tv = { 0 };
    uint64_t time_ms;

    gettimeofday(&tv, NULL);

    time_ms = (uint64_t)tv.tv_sec * 1000LL + tv.tv_usec / 1000;

    return time_ms;
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
    if ((ms > 0) && (ms < portTICK_RATE_MS)) {
        ms = portTICK_RATE_MS;
    }

    vTaskDelay(ms / portTICK_RATE_MS);
}

void HAL_Srandom(uint32_t seed)
{
    return;
}

uint32_t HAL_Random(uint32_t region)
{
    return esp_random();
}

int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *format, va_list ap)
{
    return vsnprintf(str, len, format, ap);
}

void HAL_Printf(_IN_ const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}

int HAL_GetPartnerID(char* pid_str)
{
    memset(pid_str, 0x0, PID_STRLEN_MAX);
    strcpy(pid_str, "espressif");
    return strlen(pid_str);
}

int HAL_GetModuleID(char* mid_str)
{
    memset(mid_str, 0x0, MID_STRLEN_MAX);
    strcpy(mid_str, "wroom-32");
    return strlen(mid_str);
}

char *HAL_GetChipID(_OU_ char* cid_str)
{
    memset(cid_str, 0x0, HAL_CID_LEN);
    strncpy(cid_str, "esp32", HAL_CID_LEN);
    cid_str[HAL_CID_LEN - 1] = '\0';
    return cid_str;
}

int HAL_GetDeviceID(_OU_ char* device_id)
{
    memset(device_id, 0x0, DEVICE_ID_LEN);
    HAL_Snprintf(device_id, DEVICE_ID_LEN, "%s.%s", _product_key, _device_name);
    device_id[DEVICE_ID_LEN - 1] = '\0';
    return strlen(device_id);
}

#ifdef MQTT_ID2_AUTH
int HAL_GetID2(_OU_ char* id2_str)
{
    int rc;
    uint8_t                 id2[TFS_ID2_LEN + 1] = {0};
    uint32_t                id2_len = TFS_ID2_LEN + 1;
    memset(id2_str, 0x0, TFS_ID2_LEN + 1);
    rc = tfs_get_ID2(id2, &id2_len);
    if (rc < 0) return rc;
    strncpy(id2_str, (const char*)id2, TFS_ID2_LEN);
    return strlen(id2_str);
}
#endif /**< MQTT_ID2_AUTH*/

int HAL_SetProductKey(_IN_ char* product_key)
{
    int len = strlen(product_key);
    if (len > PRODUCT_KEY_LEN) return -1;
    memset(_product_key, 0x0, PRODUCT_KEY_LEN + 1);
    strncpy(_product_key, product_key, len);
    return len;
}

int HAL_SetDeviceName(_IN_ char* device_name)
{
    int len = strlen(device_name);
    if (len > DEVICE_NAME_LEN) return -1;
    memset(_device_name, 0x0, DEVICE_NAME_LEN + 1);
    strncpy(_device_name, device_name, len);
    return len;
}

int HAL_SetDeviceSecret(_IN_ char* device_secret)
{
    int len = strlen(device_secret);
    if (len > DEVICE_SECRET_LEN) return -1;
    memset(_device_secret, 0x0, DEVICE_SECRET_LEN + 1);
    strncpy(_device_secret, device_secret, len);
    return len;
}

int HAL_SetProductSecret(_IN_ char* product_secret)
{
    int len = strlen(product_secret);
    if (len > PRODUCT_SECRET_LEN) return -1;
    memset(_product_secret, 0x0, PRODUCT_SECRET_LEN + 1);
    strncpy(_product_secret, product_secret, len);
    return len;
}

int HAL_GetProductKey(_OU_ char* product_key)
{
    memset(product_key, 0x0, PRODUCT_KEY_LEN);
    strncpy(product_key, _product_key, PRODUCT_KEY_LEN);
    return strlen(product_key);
}

int HAL_GetProductSecret(_OU_ char* product_secret)
{
    memset(product_secret, 0x0, PRODUCT_SECRET_LEN);
    strncpy(product_secret, _product_secret, PRODUCT_SECRET_LEN);
    return strlen(product_secret);
}

int HAL_GetDeviceName(_OU_ char* device_name)
{
    memset(device_name, 0x0, DEVICE_NAME_LEN);
    strncpy(device_name, _device_name, DEVICE_NAME_LEN);
    return strlen(device_name);
}

int HAL_GetDeviceSecret(_OU_ char* device_secret)
{
    memset(device_secret, 0x0, DEVICE_SECRET_LEN);
    strncpy(device_secret, _device_secret, DEVICE_SECRET_LEN);
    return strlen(device_secret);
}

int HAL_GetFirmwareVesion(_OU_ char* version)
{
    char *ver = "v2.x.2.10";
    int len = strlen(ver);
    if (len > FIRMWARE_VERSION_MAXLEN)
        return 0;
    memset(version, 0x0, FIRMWARE_VERSION_MAXLEN);
    strncpy(version, ver, FIRMWARE_VERSION_MAXLEN);
    return len;
}

void HAL_Firmware_Persistence_Start(void)
{
    return;
}

int HAL_Firmware_Persistence_Write(_IN_ char *buffer, _IN_ uint32_t length)
{
    return 0;
}

int HAL_Firmware_Persistence_Stop(void)
{
    return 0;
}
