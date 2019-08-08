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

#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_event_loop.h>
#include <esp_wifi.h>
#include <esp_log.h>

#include "dm_wrapper.h"
#include "wrappers_extra.h"

#define STA_SSID_KEY             "ssid"
#define STA_PASSWORD_KEY         "pswd"
#define NW_CONFIGURED_KEY        "nw_config"

static const char *TAG = "extra";

static system_event_cb_t hal_wifi_prov_handler;
static system_event_cb_t hal_wifi_system_cb;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t wifi_event_group;
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

static esp_err_t hal_wifi_event_loop_handler(void *ctx, system_event_t *event)
{   
    /** The other method loop event handle */
    if (hal_wifi_prov_handler) {
        hal_wifi_prov_handler(ctx, event);
    }

    /** The linkkit loop event handle */
    switch (event->event_id) {
        case SYSTEM_EVENT_AP_START:
            break;
        case SYSTEM_EVENT_AP_STOP:
            break;
        case SYSTEM_EVENT_AP_STACONNECTED:
            ESP_LOGI(TAG, "A station connected to soft-AP");
            break;
        case SYSTEM_EVENT_AP_STADISCONNECTED:
            ESP_LOGI(TAG, "A station disconnected from soft-AP");
            break;
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            /* enable ipv6 */
            tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "Got IP: %s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_AP_STA_GOT_IP6:
            ESP_LOGI(TAG, "current linklocal IP[%s]", ip6addr_ntoa(&event->event_info.got_ip6.ip6_info.ip));
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }

    /** The application loop event handle */
    if (hal_wifi_system_cb) {
        hal_wifi_system_cb(ctx, event);
    }

    return ESP_OK;
}

int HAL_Wifi_Soft_Ap_Start(uint8_t *ssid, size_t ssid_len)
{
     wifi_config_t wifi_config = {
        .ap = {
            .ssid = "",
            .ssid_len = 0,
            .max_connection = 4,
            .password = "",
            .authmode = WIFI_AUTH_OPEN
        },
    };
    memcpy(wifi_config.ap.ssid, ssid, ssid_len);
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    esp_wifi_start();
    
    return SUCCESS_RETURN;
}

int HAL_Wifi_Soft_Ap_Stop(void)
{
    ESP_LOGI(TAG, "Stopping Soft AP");
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if (mode == WIFI_MODE_AP) {
        esp_wifi_stop();
    }
    esp_wifi_set_mode(WIFI_MODE_STA);
    
    return SUCCESS_RETURN;
}

void HAL_Wifi_Register_Prov_Handler(system_event_cb_t nw_prov_event_handler)
{
    hal_wifi_prov_handler = nw_prov_event_handler;
}

void HAL_Wifi_Unregister_Prov_Handler(void)
{
    hal_wifi_prov_handler = NULL;
}

void HAL_Wifi_Register_System_Event(system_event_cb_t cb)
{
    hal_wifi_system_cb = cb;
}

int HAL_Wifi_Got_IP(size_t timeout_ms)
{
    EventBits_t uxBits;
    
    /* Wait for the callback to set the CONNECTED_BIT in the event group. */
    uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, timeout_ms / portTICK_RATE_MS);
    
    return (uxBits & CONNECTED_BIT) ? SUCCESS_RETURN : FAIL_RETURN;
}

int HAL_Wifi_Sta_Connect(const uint8_t *ssid, uint16_t ssid_len, const uint8_t *password, uint16_t password_len, size_t timeout_ms)
{
    wifi_config_t wifi_config = {0};
    wifi_mode_t mode;
    
    if (ssid) {
        memcpy(wifi_config.sta.ssid, ssid, ssid_len);
        if (password)
            memcpy(wifi_config.sta.password, password, password_len);
    } else {
        ssid_len = sizeof(wifi_config.sta.ssid);
        password_len = sizeof(wifi_config.sta.password);
        int ret = HAL_Kv_Get(STA_SSID_KEY, wifi_config.sta.ssid, &ssid_len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to get stored SSID");
            return FAIL_RETURN;
        }

        /* Even if a password is not found, it is not an error, as it could be an open network */
        ret = HAL_Kv_Get(STA_PASSWORD_KEY, wifi_config.sta.password, &password_len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to get stored Password");
            password_len = 0;
        }
    }
    
    esp_wifi_get_mode(&mode);
    if (mode == WIFI_MODE_AP) {
        esp_wifi_stop();
        esp_wifi_set_mode(WIFI_MODE_APSTA);
    } else {
        esp_wifi_set_mode(WIFI_MODE_STA);
    }

    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    return HAL_Wifi_Got_IP(timeout_ms);
}

bool HAL_Wifi_Is_Network_Configured(void)
{
    uint8_t  nw_configured = 0;
    uint16_t configured_size = sizeof(nw_configured);
    HAL_Kv_Get(NW_CONFIGURED_KEY, &nw_configured, &configured_size);
    
    return nw_configured ? true : false;
}

int HAL_Wifi_Save_Network(const uint8_t *ssid, size_t ssid_len, const uint8_t *password, size_t password_len)
{
    int ret = HAL_Kv_Set(STA_SSID_KEY, ssid, ssid_len, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "%s key store failed with %d", STA_SSID_KEY, ret);
        return FAIL_RETURN;
    }

    /* Password may be NULL. Save, only if it is given */
    if (password) {
        ret = HAL_Kv_Set(STA_PASSWORD_KEY, password, password_len, 0);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "%s key store failed with %d", STA_PASSWORD_KEY, ret);
            return FAIL_RETURN;
        }
    }
    uint8_t nw_configured = 1;
    ret = HAL_Kv_Set(NW_CONFIGURED_KEY, &nw_configured, sizeof(nw_configured), 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "%s key store failed with %d", NW_CONFIGURED_KEY, ret);
        return FAIL_RETURN;
    }

    return SUCCESS_RETURN;
}

int HAL_Wifi_Del_Network(void)
{
    HAL_Kv_Del(NW_CONFIGURED_KEY);
    HAL_Kv_Del(STA_SSID_KEY);
    HAL_Kv_Del(STA_PASSWORD_KEY);

    return SUCCESS_RETURN;
}

int HAL_Wifi_Init(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_event_loop_init(hal_wifi_event_loop_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    return SUCCESS_RETURN;
}
