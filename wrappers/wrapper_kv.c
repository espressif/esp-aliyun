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

#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_log.h"

static const char *NVS_KV = "iotkit-kv";
static const char *TAG = "wrapper_kv";


esp_err_t HAL_kv_init()
{
    static bool init_flag = false;

    if (!init_flag) {
        esp_err_t ret = nvs_flash_init();

        if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }

        ESP_ERROR_CHECK(ret);

        init_flag = true;
    }

    return ESP_OK;
}


int HAL_Kv_Del(const char *key)
{
    if(key == NULL){
        return ESP_FAIL;
    }

    HAL_kv_init();

    esp_err_t ret    = ESP_OK;
    nvs_handle handle;

    ret = nvs_open(NVS_KV, NVS_READWRITE, &handle);
    if(ret != ESP_OK){
        return ESP_FAIL;
    }

    if(strlen(key) > 15){
        char short_key[16] = {0};
        strncpy(short_key,key,15);
        ret = nvs_erase_key(handle, short_key);
    }else{
        ret = nvs_erase_key(handle, key);
    }

    if(ret != ESP_OK){
        ESP_LOGE(TAG,"nvs_erase_key key:%s,error id :%d",key,ret);
    }

    nvs_commit(handle);
    nvs_close(handle);

    return ret;
}

int HAL_Kv_Get(const char *key, void *val, int *buffer_len)
{
    if(key == NULL || val == NULL || buffer_len == NULL){
        return ESP_FAIL;
    }

    HAL_kv_init();

    esp_err_t ret  = ESP_OK;
    nvs_handle handle ;

    ret = nvs_open(NVS_KV, NVS_READONLY, &handle);
    if(ret != ESP_OK){
        return ESP_FAIL;
    }

    if(strlen(key) > 15){
        char short_key[16] = {0};
        strncpy(short_key,key,15);
        ret = nvs_get_blob(handle, short_key, val, (size_t *)buffer_len);
    }else{
        ret = nvs_get_blob(handle, key, val, (size_t *)buffer_len);
    }

    nvs_close(handle);

    return ret;
}

int HAL_Kv_Set(const char *key, const void *val, int len, int sync)
{
    if(key == NULL || val == NULL || len <= 0){
        ESP_LOGE(TAG,"HAL_Kv_Set is NULL");
        return ESP_FAIL;
    }

    HAL_kv_init();

    esp_err_t ret  = ESP_OK;
    nvs_handle handle ;

    ret = nvs_open(NVS_KV, NVS_READWRITE, &handle);

    if(ret != ESP_OK){
        ESP_LOGE(TAG,"nvs_open error");
        return ESP_FAIL;
    }
    if(strlen(key) > 15){
        char short_key[16] = {0};
        strncpy(short_key,key,15);
        ret = nvs_set_blob(handle, short_key, val, len);
    }else{
        ret = nvs_set_blob(handle, key, val, len);
    }

    nvs_commit(handle);
    nvs_close(handle);

    return ret;
}
