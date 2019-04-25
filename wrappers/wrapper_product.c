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

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "infra_defs.h"

#define NVS_KV "ALIYUN-KEY"

static const char *TAG = "wrapper_product";


esp_err_t HAL_product_init()
{
    static bool init_flag = false;

    if (!init_flag) {
        esp_err_t ret = nvs_flash_init();

        if (ret == ESP_ERR_NVS_NO_FREE_PAGES ) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }

        ESP_ERROR_CHECK(ret);

        init_flag = true;
    }
    return ESP_OK;
}

/**
 * @brief   获取设备的固件版本字符串
 *
 * @param   version : 用来存放版本字符串的数组
 * @return  写到version[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetFirmwareVersion(char *version)
{
    if(version == NULL){
        ESP_LOGE(TAG,"HAL_GetFirmwareVersion version is NULL");
        return 0;
    }

    HAL_product_init();

    esp_err_t ret = ESP_OK;
    nvs_handle handle;
    size_t read_len = IOTX_FIRMWARE_VERSION_LEN;
    ret = nvs_open(NVS_KV, NVS_READWRITE, &handle);
    if(ret != ESP_OK){
        ESP_LOGE(TAG,"HAL_SetDeviceSecret nvs_open error, ret:%d",ret);
    }else{
        nvs_get_str(handle, "ProductKey", version, (size_t *)&read_len);
        nvs_close(handle);
        strcat(version,"-");
        #ifdef CONFIG_LINKKIT_FIRMWARE_VERSION
        strcat(version, CONFIG_LINKKIT_FIRMWARE_VERSION);
        #endif
    }
    return strlen(version);
}

/**
 * @brief   设置设备的`ProductKey`, 用于标识设备的品类, 三元组之一
 *
 * @param   product_key : 用来存放ProductKey字符串的数组
 * @return  写到product_key[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_SetProductKey(char *product_key)
{
    if(product_key == NULL){
        ESP_LOGE(TAG,"HAL_SetProductKey product_key is NULL");
        return 0;
    }

    HAL_product_init();

    esp_err_t ret = ESP_OK;
    nvs_handle handle;
    ret = nvs_open(NVS_KV, NVS_READWRITE, &handle);
    if(ret != ESP_OK){
        ESP_LOGE(TAG,"HAL_SetDeviceSecret nvs_open error, ret:%d",ret);
    }else{
        ret = nvs_set_str(handle, "ProductKey", product_key);
        if(ret != ESP_OK){
            ESP_LOGW(TAG,"HAL_SetDeviceSecret nvs_set_str error, ret:%d",ret);
        }
        nvs_close(handle);
    }
    return strlen(product_key);
}

/**
 * @brief   设置设备的`DeviceName`, 用于标识设备单品的名字, 三元组之一
 *
 * @param   device_name : 用来存放DeviceName字符串的数组
 * @return  写到device_name[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_SetDeviceName(char *device_name)
{
    if(device_name == NULL){
        ESP_LOGE(TAG,"HAL_SetDeviceName device_name is NULL");
        return 0;
    }

    HAL_product_init();

    esp_err_t ret = ESP_OK;
    nvs_handle handle;
    ret = nvs_open(NVS_KV, NVS_READWRITE, &handle);
    if(ret != ESP_OK){
        ESP_LOGE(TAG,"HAL_SetDeviceSecret nvs_open error, ret:%d",ret);
    }else{
        ret = nvs_set_str(handle, "DeviceName", device_name);
        if(ret != ESP_OK){
            ESP_LOGW(TAG,"HAL_SetDeviceSecret nvs_set_str error, ret:%d",ret);
        }
        nvs_close(handle);
    }
    return strlen(device_name);
}

/**
 * @brief   设置设备的`DeviceSecret`, 用于标识设备单品的密钥, 三元组之一
 *
 * @param   device_secret : 用来存放DeviceSecret字符串的数组
 * @return  写到device_secret[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_SetDeviceSecret(char *device_secret)
{
    if(device_secret == NULL){
        ESP_LOGE(TAG,"HAL_SetDeviceSecret device_secret is NULL");
        return 0;
    }

    HAL_product_init();

    esp_err_t ret = ESP_OK;
    nvs_handle handle;
    ret = nvs_open(NVS_KV, NVS_READWRITE, &handle);
    if(ret != ESP_OK){
        ESP_LOGE(TAG,"HAL_SetDeviceSecret nvs_open error, ret:%d",ret);
    }else{
        ret = nvs_set_str(handle, "DeviceSecret", device_secret);
        if(ret != ESP_OK){
            ESP_LOGW(TAG,"HAL_SetDeviceSecret nvs_set_str error, ret:%d",ret);
        }
        nvs_close(handle);
    }
    return strlen(device_secret);
}

/**
 * @brief   设置设备的`ProductSecret`, 用于标识设备单品的密钥, 三元组之一
 *
 * @param   product_secret : 用来存放ProductSecret字符串的数组
 * @return  写到product_secret[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_SetProductSecret(char *product_secret)
{
    if(product_secret == NULL){
        ESP_LOGE(TAG,"HAL_SetProductSecret product_secret is NULL");
        return 0;
    }

    HAL_product_init();

    esp_err_t ret = ESP_OK;
    nvs_handle handle;
    ret = nvs_open(NVS_KV, NVS_READWRITE, &handle);
    if(ret != ESP_OK){
        ESP_LOGE(TAG,"HAL_SetProductSecret nvs_open error, ret:%d",ret);
    }else{
        ret = nvs_set_str(handle, "ProductSecret", product_secret);
        if(ret != ESP_OK){
            ESP_LOGW(TAG,"HAL_SetProductSecret nvs_set_str error, ret:%d",ret);
        }
        nvs_close(handle);
    }
    return strlen(product_secret);
}

/**
 * @brief   获取设备的`ProductKey`, 用于标识设备的品类, 三元组之一
 *
 * @param   product_key : 用来存放ProductKey字符串的数组
 * @return  写到product_key[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN])
{
    if(product_key == NULL){
        ESP_LOGE(TAG,"HAL_GetProductKey product_key is NULL");
        return 0;
    }

    HAL_product_init();

    esp_err_t ret = ESP_OK;
    size_t read_len = IOTX_PRODUCT_KEY_LEN;
    nvs_handle handle;
    ret = nvs_open(NVS_KV, NVS_READWRITE, &handle);
    if(ret != ESP_OK){
        ESP_LOGE(TAG,"HAL_GetProductKey nvs_open error, ret:%d",ret);
    }else{
        ret = nvs_get_str(handle, "ProductKey", product_key, (size_t *) &read_len);
        if(ret == ESP_ERR_NVS_NOT_FOUND){
            ESP_LOGW(TAG,"HAL_GetProductKey nvs_get_str not found");
            nvs_close(handle);
            #ifdef CONFIG_LINKKIT_PRODUCT_KEY
            ESP_LOGW(TAG,"HAL_GetProductKey write menuconfig config linkkit key");
            HAL_SetProductKey(CONFIG_LINKKIT_PRODUCT_KEY);
            strncpy(product_key, CONFIG_LINKKIT_PRODUCT_KEY, IOTX_PRODUCT_KEY_LEN);
            #endif
        }else if(ret != ESP_OK){
            ESP_LOGW(TAG,"HAL_GetProductKey nvs_get_str error, ret:%d",ret);
        }
    }
    ESP_LOGV(TAG,"HAL_GetProductKey :%s",product_key);
    return strlen(product_key);
}
/**
 * @brief   获取设备的`DeviceName`, 用于标识设备单品的名字, 三元组之一
 *
 * @param   device_name : 用来存放DeviceName字符串的数组
 * @return  写到device_name[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN])
{
    if(device_name == NULL){
        ESP_LOGE(TAG,"HAL_GetDeviceName device_name is NULL");
        return 0;
    }

    HAL_product_init();

    esp_err_t ret = ESP_OK;
    size_t read_len = IOTX_DEVICE_NAME_LEN;
    nvs_handle handle;
    ret = nvs_open(NVS_KV, NVS_READWRITE, &handle);
    if(ret != ESP_OK){
        ESP_LOGE(TAG,"HAL_GetDeviceName nvs_open error, ret:%d",ret);
    }else{
        ret = nvs_get_str(handle, "DeviceName", device_name, (size_t *)&read_len);
        if(ret == ESP_ERR_NVS_NOT_FOUND){
            ESP_LOGW(TAG,"HAL_GetDeviceName nvs_get_str not found");
            nvs_close(handle);
            #ifdef CONFIG_LINKKIT_DEVICE_NAME
            ESP_LOGW(TAG,"HAL_GetDeviceName write menuconfig config linkkit key");
            HAL_SetDeviceName(CONFIG_LINKKIT_DEVICE_NAME);
            strncpy(device_name, CONFIG_LINKKIT_DEVICE_NAME, IOTX_DEVICE_NAME_LEN);
            #endif
        }else if(ret != ESP_OK){
            ESP_LOGW(TAG,"HAL_GetDeviceName nvs_get_str error, ret:%d",ret);
        }
    }
    ESP_LOGV(TAG,"HAL_GetDeviceName :%s",device_name);
    return strlen(device_name);
}

/**
 * @brief   获取设备的`DeviceSecret`, 用于标识设备单品的密钥, 三元组之一
 *
 * @param   device_secret : 用来存放DeviceSecret字符串的数组
 * @return  写到device_secret[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN])
{
    if(device_secret == NULL){
        ESP_LOGE(TAG,"HAL_GetDeviceSecret device_secret is NULL");
        return 0;
    }

    HAL_product_init();

    esp_err_t ret = ESP_OK;
    size_t read_len = IOTX_DEVICE_SECRET_LEN;
    nvs_handle handle;
    ret = nvs_open(NVS_KV, NVS_READWRITE, &handle);
    if(ret != ESP_OK){
        ESP_LOGE(TAG,"HAL_GetDeviceSecret nvs_open error, ret:%d",ret);
    }else{
        ret = nvs_get_str(handle, "DeviceSecret", device_secret, (size_t *)&read_len);;
        if(ret == ESP_ERR_NVS_NOT_FOUND){
            ESP_LOGW(TAG,"HAL_GetDeviceSecret nvs_get_str not found");
            nvs_close(handle);
            #ifdef CONFIG_LINKKIT_DEVICE_SECRET
            ESP_LOGW(TAG,"HAL_GetDeviceSecret write menuconfig config linkkit key");
            HAL_SetDeviceSecret(CONFIG_LINKKIT_DEVICE_SECRET);
            strncpy(device_secret, CONFIG_LINKKIT_DEVICE_SECRET, IOTX_DEVICE_SECRET_LEN);
            #endif
        }else if(ret != ESP_OK){
            ESP_LOGW(TAG,"HAL_GetDeviceSecret nvs_get_str error, ret:%d",ret);
        }
    }
    ESP_LOGV(TAG,"HAL_GetDeviceSecret :%s",device_secret);
    return strlen(device_secret);
}

/**
 * @brief   获取设备的`ProductSecret`, 用于标识设备单品的密钥, 三元组之一
 *
 * @param   product_secret : 用来存放ProductSecret字符串的数组
 * @return  写到product_secret[]数组中的字符长度, 单位是字节(Byte)
 */
int HAL_GetProductSecret(char product_secret[IOTX_PRODUCT_SECRET_LEN])
{
    if(product_secret == NULL){
        ESP_LOGE(TAG,"HAL_GetProductSecret product_secret is NULL");
        return 0;
    }

    HAL_product_init();

    esp_err_t ret = ESP_OK;
    size_t read_len = IOTX_PRODUCT_SECRET_LEN;
    nvs_handle handle;
    ret = nvs_open(NVS_KV, NVS_READWRITE, &handle);
    if(ret != ESP_OK){
        ESP_LOGE(TAG,"HAL_GetProductSecret nvs_open error, ret:%d",ret);
    }else{
        ret = nvs_get_str(handle, "ProductSecret", product_secret, (size_t *)&read_len);
        if(ret == ESP_ERR_NVS_NOT_FOUND){
            ESP_LOGW(TAG,"HAL_GetProductSecret nvs_get_str not found");
            nvs_close(handle);
            #ifdef CONFIG_LINKKIT_PRODUCT_SECRET
            ESP_LOGW(TAG,"HAL_GetProductSecret write menuconfig config linkkit key");
            HAL_SetProductSecret(CONFIG_LINKKIT_PRODUCT_SECRET);
            strncpy(product_secret, CONFIG_LINKKIT_PRODUCT_SECRET, IOTX_PRODUCT_SECRET_LEN);
            #endif
        }else if(ret != ESP_OK){
            ESP_LOGW(TAG,"HAL_GetProductSecret nvs_get_str error, ret:%d",ret);
        }
    }
    ESP_LOGV(TAG,"HAL_GetProductSecret :%s",product_secret);
    return strlen(product_secret);
}
