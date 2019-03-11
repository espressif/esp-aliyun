/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include "sdkconfig.h"

#include <string.h>
#include "errno.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "lwip/sockets.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "iot_import.h"

#define NVS_KEY_WIFI_CONFIG "wifi_config"
#define AWSS_SPACE_NAME     "AWSS_APP"
static const char *TAG = "awss_config";

// wifi
static bool sys_net_is_ready = false;
static const int CONNECTED_BIT = BIT0;
static EventGroupHandle_t wifi_event_group = NULL;
static SemaphoreHandle_t s_sem_connect_timeout = NULL;
static system_event_cb_t s_user_wifi_event_cb = NULL;

// awss
static awss_recv_80211_frame_cb_t s_sniffer_cb = NULL;
typedef void (*wifi_sta_rx_probe_req_t)(const uint8_t *frame, int len, int rssi);
extern esp_err_t esp_wifi_set_sta_rx_probe_req(wifi_sta_rx_probe_req_t cb);
static awss_wifi_mgmt_frame_cb_t s_awss_mgmt_frame_cb = NULL;
static uint8_t s_esp_oui[3] = { 0 };

#define HAL_LOGE( format, ... ) ESP_LOGE(TAG, "[%s, %d]:" format, __func__, __LINE__, ##__VA_ARGS__)

/**
 * @brief Check the return value
 */
#define AWSS_ERROR_CHECK(con, err, format, ...) do { \
    if (con) { \
        HAL_LOGE(format , ##__VA_ARGS__); \
        if(errno) HAL_LOGE("errno: %d, errno_str: %s\n", errno, strerror(errno)); \
        return err; \
    } \
}while (0)

int esp_info_erase(const char *key)
{
    int ret = ESP_OK;
    nvs_handle handle = 0;

    AWSS_ERROR_CHECK(!key, -1, "Invalid argument");

    ret = nvs_open(AWSS_SPACE_NAME, NVS_READWRITE, &handle);
    AWSS_ERROR_CHECK(ret != ESP_OK, -1, "nvs_open ret:%x", ret);

    ret = nvs_erase_key(handle, key);
    nvs_commit(handle);
    nvs_close(handle);
    AWSS_ERROR_CHECK(ret != ESP_OK, -1, "nvs_erase_key ret:%x", ret);
    return 0;
}

ssize_t esp_info_save(const char *key, const void *value, size_t length)
{
    int ret = ESP_OK;
    nvs_handle handle = 0;

    AWSS_ERROR_CHECK(!key || !value, -1, "Invalid argument");
    
    ret = nvs_open(AWSS_SPACE_NAME, NVS_READWRITE, &handle);
    AWSS_ERROR_CHECK(ret != ESP_OK, -1, "nvs_open ret:%x", ret);

    /**
     * Reduce the number of flash writes
     */
    char *tmp = (char *)malloc(length);
    ret = nvs_get_blob(handle, key, tmp, &length);
    if ((ret == ESP_OK) && !memcmp(tmp, value, length)) {
        free(tmp);
        nvs_close(handle);
        return length;
    }
    free(tmp);

    ret = nvs_set_blob(handle, key, value, length);
    nvs_commit(handle);
    nvs_close(handle);
    AWSS_ERROR_CHECK(ret != ESP_OK, -1, "nvs_set_blob ret:%x", ret);
    return length;
}

ssize_t esp_info_load(const char *key, void *value, size_t length)
{
    int ret = ESP_OK;
    nvs_handle handle = 0;

    AWSS_ERROR_CHECK(!key || !value, -1, "Invalid argument");

    ret = nvs_open(AWSS_SPACE_NAME, NVS_READWRITE, &handle);
    AWSS_ERROR_CHECK(ret != ESP_OK, -1, "nvs_open ret:%x", ret);

    ret = nvs_get_blob(handle, key, value, &length);
    nvs_close(handle);

    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG,"No data storage,the load data is empty");
        return -1;
    }
    AWSS_ERROR_CHECK(ret != ESP_OK, -1, "nvs_get_blob ret:%x", ret);
    return length;
}

/**
 * @brief   获取`smartconfig`服务的安全等级
 *
 * @param None.
 * @return The security level:
   @verbatim
    0: open (no encrypt)
    1: aes256cfb with default aes-key and aes-iv
    2: aes128cfb with default aes-key and aes-iv
    3: aes128cfb with aes-key per product and aes-iv = 0
    4: aes128cfb with aes-key per device and aes-iv = 0
    5: aes128cfb with aes-key per manufacture and aes-iv = 0
    others: invalid
   @endverbatim
 * @see None.
 */
int HAL_Awss_Get_Encrypt_Type()
{
    return 3;
}

/**
 * @brief    Get Security level for wifi configuration with connection.
 *           Used for AP solution of router and App.
 *
 * @param None.
 * @return The security level:
   @verbatim
    3: aes128cfb with aes-key per product and aes-iv = random
    4: aes128cfb with aes-key per device and aes-iv = random
    5: aes128cfb with aes-key per manufacture and aes-iv = random
    others: invalid
   @endverbatim
 * @see None.
 */
int HAL_Awss_Get_Conn_Encrypt_Type()
{
    return 4;
}

/**
 * @brief   获取Wi-Fi网口的MAC地址, 格式应当是"XX:XX:XX:XX:XX:XX"
 *
 * @param   mac_str : 用于存放MAC地址字符串的缓冲区数组
 * @return  指向缓冲区数组起始位置的字符指针
 */
char *HAL_Wifi_Get_Mac(_OU_ char mac_str[HAL_MAC_LEN])
{
    uint8_t mac[6] = {0};

    if (mac_str == NULL) {
        return NULL;
    }

    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, mac));
    snprintf(mac_str, HAL_MAC_LEN, MACSTR, MAC2STR(mac));
    return mac_str;
}

/**
 * @brief   获取配网服务(`AWSS`)的超时时间长度, 单位是毫秒
 *
 * @return  超时时长, 单位是毫秒
 * @note    推荐时长是60,0000毫秒
 */
int HAL_Awss_Get_Timeout_Interval_Ms(void)
{
    return 30 * 60 * 1000;
}

/**
 * @brief   获取在每个信道(`channel`)上扫描的时间长度, 单位是毫秒
 *
 * @return  时间长度, 单位是毫秒
 * @note    推荐时长是200毫秒到400毫秒
 */
int HAL_Awss_Get_Channelscan_Interval_Ms(void)
{
    return 250;
}

/**
 * @brief   802.11帧的处理函数, 可以将802.11 Frame传递给这个函数
 *
 * @param[in] buf @n 80211 frame buffer, or pointer to struct ht40_ctrl
 * @param[in] length @n 80211 frame buffer length
 * @param[in] link_type @n AWSS_LINK_TYPE_NONE for most rtos HAL,
 *              and for linux HAL, do the following step to check
 *              which header type the driver supported.
     @verbatim
                a) iwconfig wlan0 mode monitor    #open monitor mode
                b) iwconfig wlan0 channel 6    #switch channel 6
                c) tcpdump -i wlan0 -s0 -w file.pacp    #capture 80211 frame
    & save d) open file.pacp with wireshark or omnipeek check the link header
    type and fcs included or not
    @endverbatim
    * @param[in] with_fcs @n 80211 frame buffer include fcs(4 byte) or not
    * @param[in] rssi @n rssi of packet
    */
typedef int (*awss_recv_80211_frame_cb_t)(char *buf, int length,
                                            enum AWSS_LINK_TYPE link_type,
                                            int with_fcs, signed char rssi);

typedef struct hal_wifi_link_info_s {
    int8_t rssi; /* rssi value of received packet */
} hal_wifi_link_info_t;

typedef void (*awss_wifi_mgmt_frame_cb_t)(_IN_ uint8_t *   buffer,
                                            _IN_ int         len,
                                            _IN_ signed char rssi_dbm,
                                            _IN_ int         buffer_type);

static void IRAM_ATTR wifi_sniffer_cb(void *recv_buf, wifi_promiscuous_pkt_type_t type)
{
    int with_fcs = 0;
    int link_type = AWSS_LINK_TYPE_NONE;
    uint16_t len = 0;
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)recv_buf;
    hal_wifi_link_info_t info;

    if (type != WIFI_PKT_DATA && type != WIFI_PKT_MGMT) {
        return;
    }

    info.rssi = pkt->rx_ctrl.rssi;

#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    uint8_t total_num = 1, count;
    uint16_t seq_buf;
    len = pkt->rx_ctrl.sig_mode ? pkt->rx_ctrl.HT_length : pkt->rx_ctrl.legacy_length;

    if (pkt->rx_ctrl.aggregation) {
        total_num = pkt->rx_ctrl.ampdu_cnt;
    }

    for (count = 0; count < total_num; count++) {
        if (total_num > 1) {
            len = *(uint16_t *)(pkt->payload + 40 + 2 * count);
        }

        if (type == WIFI_PKT_MISC && pkt->rx_ctrl.aggregation == 1) {
            len -= 4;
        }

        if (s_sniffer_cb) {
            s_sniffer_cb((char *)pkt->payload, len - 4, link_type, with_fcs, info.rssi);
        }

        if (total_num > 1) {
            seq_buf = *(uint16_t *)(pkt->payload + 22) >> 4;
            seq_buf++;
            *(uint16_t *)(pkt->payload + 22) = (seq_buf << 4) | (*(uint16_t *)(pkt->payload + 22) & 0xF);
        }
    }
#else
    if (s_sniffer_cb) {
        len = pkt->rx_ctrl.sig_len;
    	s_sniffer_cb((char *)pkt->payload, len - 4, link_type, with_fcs, info.rssi);
    }
#endif
}

/**
 * @brief   设置Wi-Fi网卡工作在监听(Monitor)模式, 并在收到802.11帧的时候调用被传入的回调函数
 *
 * @param[in] cb @n A function pointer, called back when wifi receive a frame.
 */
void HAL_Awss_Open_Monitor(_IN_ awss_recv_80211_frame_cb_t cb)
{
    if (cb == NULL) {
        return;
    }

    s_sniffer_cb = cb;
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(0));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(wifi_sniffer_cb));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(1));
    ESP_ERROR_CHECK(esp_wifi_set_channel(6, 0));
    ESP_LOGI(TAG, "wifi running at monitor mode");
}

/**
 * @brief   设置Wi-Fi网卡离开监听(Monitor)模式, 并开始以站点(Station)模式工作
 */
void HAL_Awss_Close_Monitor(void)
{
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(0));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    s_sniffer_cb = NULL;
    ESP_LOGI(TAG, "close wifi monitor mode, and set running at station mode");
}

/**
 * @brief   设置Wi-Fi网卡切换到指定的信道(channel)上
 *
 * @param[in] primary_channel @n Primary channel.
 * @param[in] secondary_channel @n Auxiliary channel if 40Mhz channel is supported, currently
 *              this param is always 0.
 * @param[in] bssid @n A pointer to wifi BSSID on which awss lock the channel, most HAL
 *              may ignore it.
 */
void HAL_Awss_Switch_Channel(
            _IN_ char primary_channel,
            _IN_OPT_ char secondary_channel,
            _IN_OPT_ uint8_t bssid[ETH_ALEN])
{
    // TODO: consider country code when set to limited channel
    esp_wifi_set_channel(primary_channel, secondary_channel);
}

/**
 * @brief check system network is ready(get ip address) or not.
 *
 * @param None.
 * @return 0, net is not ready; 1, net is ready.
 * @see None.
 * @note None.
 */
int HAL_Sys_Net_Is_Ready()
{
    return sys_net_is_ready;
}

uint32_t HAL_Wait_Net_Ready(uint32_t block_time_tick)
{
    if(wifi_event_group == NULL) {
        ESP_LOGE(TAG, "wifi_event_group not init");
        return 0;
    }

    if (block_time_tick == 0) {
        return (uint32_t)xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    } else {
        return (uint32_t)xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, block_time_tick);
    }
}

void esp_init_wifi_event_group()
{   
    if(wifi_event_group) {
        return;
    }
    wifi_event_group = xEventGroupCreate();
}

void set_user_wifi_event_cb(system_event_cb_t cb)
{
    s_user_wifi_event_cb = cb;
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
    case SYSTEM_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG,"SYSTEM_EVENT_STA_CONNECTED");
        break;

    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG,"SYSTEM_EVENT_STA_START");
        sys_net_is_ready = false;
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        sys_net_is_ready = true;
        ESP_LOGI(TAG,"SYSTEM_EVENT_STA_GOT_IP");
        if(wifi_event_group) {
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        }

        xSemaphoreGive(s_sem_connect_timeout);
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        sys_net_is_ready = false;
        ESP_LOGW(TAG,"SYSTEM_EVENT_STA_DISCONNECTED");
        if(wifi_event_group) {
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        }

        esp_wifi_connect();
        break;

    default:
        break;
    }

    // call user wifi event
    if(s_user_wifi_event_cb) {
        (*s_user_wifi_event_cb)(ctx, event);
    }

    return 0;
}
/**
 * @brief   要求Wi-Fi网卡连接指定热点(Access Point)的函数
 *
 * @param[in] connection_timeout_ms @n AP connection timeout in ms or HAL_WAIT_INFINITE
 * @param[in] ssid @n AP ssid
 * @param[in] passwd @n AP passwd
 * @param[in] auth @n optional(AWSS_AUTH_TYPE_INVALID), AP auth info
 * @param[in] encry @n optional(AWSS_ENC_TYPE_INVALID), AP encry info
 * @param[in] bssid @n optional(NULL or zero mac address), AP bssid info
 * @param[in] channel @n optional, AP channel info
 * @return
   @verbatim
     = 0: connect AP & DHCP success
     = -1: connect AP or DHCP fail/timeout
   @endverbatim
 * @see None.
 * @note
 *      If the STA connects the old AP, HAL should disconnect from the old AP firstly.
 *      If bssid specifies the dest AP, HAL should use bssid to connect dest AP.
 */
int HAL_Awss_Connect_Ap(
            _IN_ uint32_t connection_timeout_ms,
            _IN_ char ssid[HAL_MAX_SSID_LEN],
            _IN_ char passwd[HAL_MAX_PASSWD_LEN],
            _IN_OPT_ enum AWSS_AUTH_TYPE auth,
            _IN_OPT_ enum AWSS_ENC_TYPE encry,
            _IN_OPT_ uint8_t bssid[ETH_ALEN],
            _IN_OPT_ uint8_t channel)
{
    wifi_config_t wifi_config;
    int ret = 0;

    if (s_sem_connect_timeout == NULL) {
        s_sem_connect_timeout = xSemaphoreCreateBinary();
        esp_event_loop_set_cb(event_handler, NULL);
    }

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, passwd, sizeof(wifi_config.sta.password));

    ESP_LOGI(TAG,"ap ssid: %s, password: %s", wifi_config.sta.ssid, wifi_config.sta.password);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ret = xSemaphoreTake(s_sem_connect_timeout, connection_timeout_ms / portTICK_RATE_MS);
    if (ret == pdFALSE) {   // connect timeout
        return -1; 
    }

    if (!strcmp(ssid, "aha")) {
        return 0;
    }

    ret = esp_info_save(NVS_KEY_WIFI_CONFIG, &wifi_config, sizeof(wifi_config_t));
    AWSS_ERROR_CHECK(ret < 0, -1, "information save failed");
    return 0;
}

/**
 * @brief   在当前信道(channel)上以基本数据速率(1Mbps)发送裸的802.11帧(raw 802.11 frame)
 *
 * @param[in] type @n see enum HAL_Awss_frame_type, currently only FRAME_BEACON
 *                      FRAME_PROBE_REQ is used
 * @param[in] buffer @n 80211 raw frame, include complete mac header & FCS field
 * @param[in] len @n 80211 raw frame length
 * @return
   @verbatim
   =  0, send success.
   = -1, send failure.
   = -2, unsupported.
   @endverbatim
 * @see None.
 * @note awss use this API send raw frame in wifi monitor mode & station mode
 */
int HAL_Wifi_Send_80211_Raw_Frame(_IN_ enum HAL_Awss_Frame_Type type,
                                  _IN_ uint8_t *buffer, _IN_ int len)
{
    if (buffer) {
        return -2;
    }
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    int ret = esp_wifi_send_pkt_freedom(buffer, len, true);
#else
    extern esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
    int ret = esp_wifi_80211_tx(ESP_IF_WIFI_STA, buffer, len, true);
#endif
    AWSS_ERROR_CHECK(ret != 0, -1, "esp_wifi_80211_tx, ret: 0x%x", ret);
    return 0;
}

/**
 * @brief   在站点(Station)模式下使能或禁用对管理帧的过滤
 *
 * @param[in] filter_mask @n see mask macro in enum HAL_Awss_frame_type,
 *                      currently only FRAME_PROBE_REQ_MASK & FRAME_BEACON_MASK is used
 * @param[in] vendor_oui @n oui can be used for precise frame match, optional
 * @param[in] callback @n see awss_wifi_mgmt_frame_cb_t, passing 80211
 *                      frame or ie to callback. when callback is NULL
 *                      disable sniffer feature, otherwise enable it.
 * @return
   @verbatim
   =  0, success
   = -1, fail
   = -2, unsupported.
   @endverbatim
 * @see None.
 * @note awss use this API to filter specific mgnt frame in wifi station mode
 */

static void wifi_sta_rx_probe_req(const uint8_t *frame, int len, int rssi)
{
    vendor_ie_data_t *awss_ie_info = (vendor_ie_data_t *)(frame + 60);
    if (awss_ie_info->element_id == WIFI_VENDOR_IE_ELEMENT_ID && awss_ie_info->length != 67 && !memcmp(awss_ie_info->vendor_oui, s_esp_oui, 3)) {
        if (awss_ie_info->vendor_oui_type == 171) {
            ESP_LOGW(TAG,"frame is no support, awss_ie_info->type: %d", awss_ie_info->vendor_oui_type);
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
    int ret = ESP_OK;
    if (callback == NULL){
        return -2;
    }

    AWSS_ERROR_CHECK(filter_mask != (FRAME_PROBE_REQ_MASK | FRAME_BEACON_MASK), -2, "frame is no support, frame: 0x%x", filter_mask);
    
    s_awss_mgmt_frame_cb = callback;
    memcpy(s_esp_oui, vendor_oui, sizeof(s_esp_oui));
    ret = esp_wifi_set_sta_rx_probe_req(wifi_sta_rx_probe_req);
    AWSS_ERROR_CHECK(ret != 0, -1, "esp_wifi_set_sta_rx_probe_req, ret: %d", ret);
    return 0;
}

/**
 * @brief   启动一次Wi-Fi的空中扫描(Scan)
 *
 * @param[in] cb @n pass ssid info(scan result) to this callback one by one
 * @return 0 for wifi scan is done, otherwise return -1
 * @see None.
 * @note
 *      This API should NOT exit before the invoking for cb is finished.
 *      This rule is something like the following :
 *      HAL_Wifi_Scan() is invoked...
 *      ...
 *      for (ap = first_ap; ap <= last_ap; ap = next_ap){
 *        cb(ap)
 *      }
 *      ...
 *      HAL_Wifi_Scan() exit...
 */
int HAL_Wifi_Scan(awss_wifi_scan_result_cb_t cb)
{
    uint16_t wifi_ap_num = 0;
    wifi_ap_record_t *ap_info = NULL;
    wifi_scan_config_t scan_config;

    memset(&scan_config, 0, sizeof(scan_config));
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&wifi_ap_num));

    ESP_LOGI(TAG,"ap number: %d", wifi_ap_num);
    ap_info = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * wifi_ap_num);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&wifi_ap_num, ap_info));
    for (int i = 0; i < wifi_ap_num; ++i) {
        cb((char *)ap_info[i].ssid, (uint8_t *)ap_info[i].bssid, ap_info[i].authmode, AWSS_ENC_TYPE_INVALID,
           ap_info[i].primary, ap_info[i].rssi, 1);
    }
    ESP_ERROR_CHECK(esp_wifi_scan_stop());
    free(ap_info);
    return 0;
}

/**
 * @brief   获取所连接的热点(Access Point)的信息
 *
 * @param[out] ssid: array to store ap ssid. It will be null if ssid is not required.
 * @param[out] passwd: array to store ap password. It will be null if ap password is not required.
 * @param[out] bssid: array to store ap bssid. It will be null if bssid is not required.
 * @return
   @verbatim
     = 0: succeeded
     = -1: failed
   @endverbatim
 * @see None.
 * @note
 *     If the STA dosen't connect AP successfully, HAL should return -1 and not touch the ssid/passwd/bssid buffer.
 */
int HAL_Wifi_Get_Ap_Info(
            _OU_ char ssid[HAL_MAX_SSID_LEN],
            _OU_ char passwd[HAL_MAX_PASSWD_LEN],
            _OU_ uint8_t bssid[ETH_ALEN])
{
    int ret = 0;
    wifi_ap_record_t ap_info;
    wifi_config_t wifi_config;

    memset(&ap_info, 0, sizeof(wifi_ap_record_t));
    ESP_ERROR_CHECK(esp_wifi_sta_get_ap_info(&ap_info));
    if (ssid) {
        memcpy(ssid, ap_info.ssid, HAL_MAX_SSID_LEN);
    }

    if (bssid) {
        memcpy(bssid, ap_info.bssid, ETH_ALEN);
    }

    if (!passwd) {
        return 0;
    }

    ret = esp_info_load(NVS_KEY_WIFI_CONFIG, &wifi_config, sizeof(wifi_config_t));
    if (ret > 0 && !memcmp(ap_info.ssid, wifi_config.sta.ssid, strlen((char *)ap_info.ssid))) {
        memcpy(passwd, wifi_config.sta.password, HAL_MAX_PASSWD_LEN);
        ESP_LOGI(TAG,"wifi passwd: %s", passwd);
    } else {
        memset(passwd, 0, HAL_MAX_PASSWD_LEN);
    }
    return 0;
}
