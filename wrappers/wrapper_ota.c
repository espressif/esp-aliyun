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

#include "infra_types.h"
#include "infra_defs.h"

#include "esp_log.h"
#include "esp_ota_ops.h"

#include "ota_wrapper.h"

typedef struct {
    const esp_partition_t     *partition;
    esp_ota_handle_t    handle;
    esp_err_t           write_err;
} hal_ota_t;

static hal_ota_t *s_ota_handle;

static const char *TAG = "ota";

void HAL_Firmware_Persistence_Start(void)
{
    if (s_ota_handle) {
        ESP_LOGE(TAG, "OTA is running...");
        return;
    }

    do {
        esp_err_t err;
        esp_ota_handle_t update_handle = 0;
        const esp_partition_t *update_partition = NULL;

        ESP_LOGI(TAG, "Starting OTA...");
        update_partition = esp_ota_get_next_update_partition(NULL);

        if (update_partition == NULL) {
            ESP_LOGE(TAG, "Passive OTA partition not found");
            break;
        }

        ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
                 update_partition->subtype, update_partition->address);

        err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);

        if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_begin failed, error=0x%x", err);
            break;
        }

        ESP_LOGI(TAG, "esp_ota_begin succeeded");
        ESP_LOGI(TAG, "Please Wait. This may take time");

        s_ota_handle = (hal_ota_t *)HAL_Malloc(sizeof(hal_ota_t));

        if (!s_ota_handle) {
            ESP_LOGE(TAG, "No space for ota handle");
            break;;
        }

        s_ota_handle->partition = update_partition;
        s_ota_handle->handle    = update_handle;
    } while (0);
}

int HAL_Firmware_Persistence_Stop(void)
{
    if (!s_ota_handle) {
        ESP_LOGE(TAG, "OTA doesn't start");
        return NULL_VALUE_ERROR;
    }

    bool err_flag = true;

    do {
        esp_err_t err = esp_ota_end(s_ota_handle->handle);

        if (s_ota_handle->write_err != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_write failed! err=0x%x", s_ota_handle->write_err);
            break;
        } else if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_end failed! err=0x%x. Image is invalid", err);
            break;
        }

        err = esp_ota_set_boot_partition(s_ota_handle->partition);

        if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
            break;
        }

        ESP_LOGI(TAG, "esp_ota_set_boot_partition succeeded");

        err_flag = false;
    } while (0);

    HAL_Free(s_ota_handle);
    s_ota_handle = NULL;

    return err_flag ? FAIL_RETURN : SUCCESS_RETURN;
}

int HAL_Firmware_Persistence_Write(char *buffer, uint32_t length)
{
    if (!s_ota_handle) {
        ESP_LOGE(TAG, "OTA doesn't start");
        return NULL_VALUE_ERROR;
    }

    s_ota_handle->write_err = esp_ota_write(s_ota_handle->handle, (const void *) buffer, length);

    if (s_ota_handle->write_err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_write failed, err=0x%x", s_ota_handle->write_err);
        return FAIL_RETURN;
    }

    return length;
}