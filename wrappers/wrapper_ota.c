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

#include "sdkconfig.h"
#ifndef CONFIG_MESH_SUPPORT_ALIYUN_LINKKIT

#include "infra_types.h"
#include "infra_defs.h"

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "ota_wrapper.h"

static const char *TAG   = "wrapper_ota";

typedef struct {
    esp_ota_handle_t handle;   /**< OTA handle */
    const esp_partition_t *partition; /**< Pointer to partition structure obtained using
                                           esp_partition_find_first or esp_partition_get. */
    esp_err_t status;  /**< Upgrade status */
} upgrade_config_t;

static upgrade_config_t *g_upgrade_config = NULL;

/**
 * @brief initialize a firmware upgrade.
 *
 * @param None
 * @return None.
 * @see None.
 * @note None.
 */

void HAL_Firmware_Persistence_Start(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_partition_t *update  = esp_ota_get_next_update_partition(NULL);

    if(running == NULL || update == NULL){
        ESP_LOGE(TAG,"No partition is found or flash read operation failed");
        return;
    }

    ESP_LOGI(TAG,"Running partition, label: %s, type: 0x%x, subtype: 0x%x, address: 0x%x",
             running->label, running->type, running->subtype, running->address);
    ESP_LOGI(TAG,"Update partition, label: %s, type: 0x%x, subtype: 0x%x, address: 0x%x",
             update->label, update->type, update->subtype, update->address);

    if (!g_upgrade_config) {
        g_upgrade_config = calloc(1, sizeof(upgrade_config_t));
    }

    g_upgrade_config->partition           = update;
    g_upgrade_config->status              = esp_ota_begin(update, OTA_SIZE_UNKNOWN,
                                            &g_upgrade_config->handle);

    if(g_upgrade_config->status != ESP_OK){
        ESP_LOGE(TAG,"esp_ota_begin failed");
    }
}

/**
 * @brief save firmware upgrade data to flash.
 *
 * @param[in] buffer: @n A pointer to a buffer to save data.
 * @param[in] length: @n The length, in bytes, of the data pointed to by the buffer parameter.
 * @return 0, Save success; -1, Save failure.
 * @see None.
 * @note None.
 */

int HAL_Firmware_Persistence_Write(char *buffer, uint32_t length)
{
    if(g_upgrade_config == NULL ){
        ESP_LOGE(TAG,"upgrade firmware is not initialized");
        return ESP_FAIL;
    }

    esp_err_t ret = ESP_OK;
    ret = esp_ota_write( g_upgrade_config->handle, (const void *)buffer, length);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "HAL_Firmware_Persistence_Write failed! ret=0x%2x", ret);
        return ESP_FAIL;
    }
    return length;
}

/**
 * @brief indicate firmware upgrade data complete, and trigger data integrity checking,
     and then reboot the system.
 *
 * @param None.
 * @return 0: Success; -1: Failure.
 * @see None.
 * @note None.
 */

int HAL_Firmware_Persistence_Stop(void)
{
    if(g_upgrade_config == NULL ){
        ESP_LOGE(TAG,"upgrade firmware is not initialized");
        return ESP_FAIL;
    }

    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

    g_upgrade_config->status = esp_ota_end(g_upgrade_config->handle);

    if(g_upgrade_config->status != ESP_OK){
        ESP_LOGE(TAG,"esp_ota_end failed");
        return ESP_FAIL;
    }

    g_upgrade_config->status = esp_ota_set_boot_partition(update_partition);

    if(g_upgrade_config->status != ESP_OK){
        ESP_LOGE(TAG,"esp_ota_set_boot_partition failed! ret=0x%x",g_upgrade_config->status);
        return ESP_FAIL;
    }
    free(g_upgrade_config);
    esp_restart();
    return ESP_OK;
}

#endif