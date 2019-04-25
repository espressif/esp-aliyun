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

#include <arpa/inet.h>
#include <string.h>

#include "iot_import_awss.h"

#include "esp_log.h"
#include "esp_wifi.h"

static const char *TAG = "wrapper_wifi.c";

static awss_wifi_mgmt_frame_cb_t s_awss_mgmt_frame_cb = NULL;
static uint8_t s_esp_oui[3] = { 0 };

typedef void (*wifi_sta_rx_probe_req_t)(const uint8_t *frame, int len, int rssi);
extern esp_err_t esp_wifi_set_sta_rx_probe_req(wifi_sta_rx_probe_req_t cb);

static void wifi_sta_rx_probe_req(const uint8_t *frame, int len, int rssi)
{
    vendor_ie_data_t *awss_ie_info = (vendor_ie_data_t *)(frame + 60);

    if (awss_ie_info->element_id == WIFI_VENDOR_IE_ELEMENT_ID && awss_ie_info->length != 67 && !memcmp(awss_ie_info->vendor_oui, s_esp_oui, 3)) {
        if (awss_ie_info->vendor_oui_type == 171) {
            ESP_LOGW(TAG, "frame is no support, awss_ie_info->type: %d", awss_ie_info->vendor_oui_type);
            return;
        }

        s_awss_mgmt_frame_cb((uint8_t *)awss_ie_info, awss_ie_info->length + 2, rssi, 1);
    }
}

int HAL_Wifi_Enable_Mgmt_Frame_Filter(
    _IN_ uint32_t filter_mask,
    _IN_OPT_ uint8_t vendor_oui[3],
    _IN_ awss_wifi_mgmt_frame_cb_t callback)
{
    if (!callback || filter_mask != (FRAME_PROBE_REQ_MASK | FRAME_BEACON_MASK)) {
        return NULL_VALUE_ERROR;
    }

    s_awss_mgmt_frame_cb = callback;
    memcpy(s_esp_oui, vendor_oui, sizeof(s_esp_oui));
    esp_err_t ret = esp_wifi_set_sta_rx_probe_req(wifi_sta_rx_probe_req);

    return ret == ESP_OK ? SUCCESS_RETURN : FAIL_RETURN;
}

int HAL_Wifi_Get_Ap_Info(char ssid[HAL_MAX_SSID_LEN], char passwd[HAL_MAX_PASSWD_LEN], uint8_t bssid[ETH_ALEN])
{
    esp_err_t ret = ESP_FAIL;
    wifi_ap_record_t ap_info;

    do {
        memset(&ap_info, 0, sizeof(wifi_ap_record_t));
        ret = esp_wifi_sta_get_ap_info(&ap_info);

        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Get AP info fail, err=0x%x", ret);
            break;
        }

        if (ssid) {
            memcpy(ssid, ap_info.ssid, HAL_MAX_SSID_LEN);
        }

        if (bssid) {
            memcpy(bssid, ap_info.bssid, ETH_ALEN);
        }

        if (ap_info.authmode != WIFI_AUTH_OPEN && passwd) {

        }
    } while (0);

    return (ret == ESP_OK) ? SUCCESS_RETURN : FAIL_RETURN;
}

uint32_t HAL_Wifi_Get_IP(char ip_str[NETWORK_ADDR_LEN], const char *ifname)
{
    esp_err_t ret = 0;
    tcpip_adapter_ip_info_t info;
    wifi_mode_t mode;

    ret = esp_wifi_get_mode(&mode);

    if (ret != ESP_OK) {
        return 0;
    }

    ret = tcpip_adapter_get_ip_info((mode == WIFI_MODE_STA) ? TCPIP_ADAPTER_IF_STA : TCPIP_ADAPTER_IF_AP, &info);

    if (ret != ESP_OK) {
        return 0;
    }

    memcpy(ip_str, inet_ntoa(info.ip.addr), NETWORK_ADDR_LEN);

    return info.ip.addr;
}

char *HAL_Wifi_Get_Mac(char mac_str[HAL_MAC_LEN])
{
    esp_err_t ret = 0;
    uint8_t mac[6] = {0};
    wifi_mode_t mode;

    ret = esp_wifi_get_mode(&mode);

    if (ret != ESP_OK) {
        return NULL;
    }

    ret = esp_wifi_get_mac((mode == WIFI_MODE_AP) ? WIFI_MODE_AP - 1 : WIFI_MODE_STA - 1, mac);

    if (ret != ESP_OK) {
        return NULL;
    }

    snprintf(mac_str, HAL_MAC_LEN, MACSTR, MAC2STR(mac));
    return (char *)mac_str;
}

int HAL_Wifi_Scan(awss_wifi_scan_result_cb_t cb)
{
    printf("here %s\n", __func__);
    return (int)1;
}

int HAL_Wifi_Send_80211_Raw_Frame(_IN_ enum HAL_Awss_Frame_Type type,
                                  _IN_ uint8_t *buffer, _IN_ int len)
{
    esp_err_t ret = ESP_OK;

    if (!buffer) {
        return NULL_VALUE_ERROR;
    }

    ret = esp_wifi_80211_tx(ESP_IF_WIFI_STA, buffer, len, true);

    return (ret == ESP_OK) ? SUCCESS_RETURN : FAIL_RETURN;
}
