/*
 * ESPRSSIF MIT License
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_common.h"
#include "esp_wifi.h"
#include "esp_system.h"

#include "esp8266/ets_sys.h"
#include "apps/sntp.h"

#include "iot_import.h"
#include "iot_export.h"
#include "cJSON.h"

#include "aliyun_config.h"
#include "aliyun_ota.h"

static int got_ip_flag = 0;
/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;
        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type) {
    case IOTX_MQTT_EVENT_UNDEF:
        EXAMPLE_TRACE("undefined event occur.");
        break;

    case IOTX_MQTT_EVENT_DISCONNECT:
        EXAMPLE_TRACE("MQTT disconnect.");
        break;

    case IOTX_MQTT_EVENT_RECONNECT:
        EXAMPLE_TRACE("MQTT reconnect.");
        break;

    case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
        EXAMPLE_TRACE("subscribe success, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
        EXAMPLE_TRACE("subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
        EXAMPLE_TRACE("subscribe nack, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
        EXAMPLE_TRACE("unsubscribe success, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
        EXAMPLE_TRACE("unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
        EXAMPLE_TRACE("unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
        EXAMPLE_TRACE("publish success, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
        EXAMPLE_TRACE("publish timeout, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_PUBLISH_NACK:
        EXAMPLE_TRACE("publish nack, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_PUBLISH_RECVEIVED:
        EXAMPLE_TRACE("topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                      topic_info->topic_len,
                      topic_info->ptopic,
                      topic_info->payload_len,
                      topic_info->payload);
        break;

    default:
        EXAMPLE_TRACE("Should NOT arrive here.");
        break;
    }
}

static void _demo_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    // print topic name and topic message
    EXAMPLE_TRACE("----");
    EXAMPLE_TRACE("Topic: '%.*s' (Length: %d)",
                  ptopic_info->topic_len,
                  ptopic_info->ptopic,
                  ptopic_info->topic_len);
    print_debug(ptopic_info->ptopic, ptopic_info->topic_len, "topic");
    print_debug(ptopic_info->payload,ptopic_info->payload_len, "payload");
    EXAMPLE_TRACE("Payload: '%.*s' (Length: %d)",
                  ptopic_info->payload_len,
                  ptopic_info->payload,
                  ptopic_info->payload_len);
    EXAMPLE_TRACE("----");
}

int mqtt_client(void)
{
    int rc = 0, msg_len, cnt = 0;
    void *pclient;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    iotx_mqtt_topic_info_t topic_msg;
    char msg_pub[128];
    char *msg_buf = NULL, *msg_readbuf = NULL;
    printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    if (NULL == (msg_buf = (char *)HAL_Malloc(MSG_LEN_MAX))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }
    printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    if (NULL == (msg_readbuf = (char *)HAL_Malloc(MSG_LEN_MAX))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }
    printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    /* Device AUTH */
    printf("\n**************\nPRODUCT_KEY:%s \nDEVICE_NAME:%s\nDEVICE_SECRET:%s\n*****************\n",PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET);
    if (0 != IOT_SetupConnInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, (void **)&pconn_info)) {
        EXAMPLE_TRACE("AUTH request failed!");
        rc = -1;
        goto do_exit;
    }
    printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port = pconn_info->port;
    mqtt_params.host = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username = pconn_info->username;
    mqtt_params.password = pconn_info->password;
    mqtt_params.pub_key = pconn_info->pub_key;

    mqtt_params.request_timeout_ms = 1000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 1000;
    mqtt_params.pread_buf = msg_readbuf;
    mqtt_params.read_buf_size = MSG_LEN_MAX;
    mqtt_params.pwrite_buf = msg_buf;
    mqtt_params.write_buf_size = MSG_LEN_MAX;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;

    /* Construct a MQTT client with specify parameter */
    printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    pclient = IOT_MQTT_Construct(&mqtt_params);
    printf("file:%s function:%s line:%d heap size:%d pClient:%p\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size(), pclient);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }
    printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
#if 1
    xTaskCreate(ota_main, "ota_main", 2048, pclient, 5, NULL);
    while(1){
        vTaskDelay(2000 / portTICK_RATE_MS);
        printf("[heap check]file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    }
#endif


    printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    /* Subscribe the specific topic */
//    rc = IOT_MQTT_Subscribe(pclient, TOPIC_DATA, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    rc = IOT_MQTT_Subscribe(pclient, TOPIC_DATA, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);

    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }

//  receive relay MQTT message
#if 0
    rc = IOT_MQTT_Subscribe(pclient, TOPIC_RELAY, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }
#endif

    HAL_SleepMs(1000);

    /* Initialize topic information */
    memset(&topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    strcpy(msg_pub, "message: hello! start!");

    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;
    topic_msg.payload = (void *)msg_pub;
    topic_msg.payload_len = strlen(msg_pub);

    while (1) {
        if(got_ip_flag){
            msg_len = snprintf(msg_pub, sizeof(msg_pub), "{\"attr_name\":\"temperature\", \"attr_value\":\"%d\"}", cnt);
            if (msg_len < 0) {
                EXAMPLE_TRACE("Error occur! Exit program");
                rc = -1;
                break;
            }

            topic_msg.payload = (void *)msg_pub;
            topic_msg.payload_len = msg_len;

            rc = IOT_MQTT_Publish(pclient, TOPIC_DATA, &topic_msg);
            //printf("file:%s function:%s line:%d heap size:%d rc:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size(), rc);
            if (rc < 0) {
                EXAMPLE_TRACE("error occur when publish");
                rc = -1;
                break;
            }
    #ifdef MQTT_ID2_CRYPTO
            EXAMPLE_TRACE("packet-id=%u, publish topic msg='0x%02x%02x%02x%02x'...",
                          (uint32_t)rc,
                          msg_pub[0], msg_pub[1], msg_pub[2], msg_pub[3]
                         );
    #else
            EXAMPLE_TRACE("packet-id=%u, publish topic msg=%s\n", (uint32_t)rc, msg_pub);
    #endif

            /* handle the MQTT packet received from TCP or SSL connection */
            IOT_MQTT_Yield(pclient, 200);

            HAL_SleepMs(3000);
        }else{

            break;
        }
        /* Generate topic message */
    }
    //printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    //printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    IOT_MQTT_Unsubscribe(pclient, TOPIC_DATA);
    IOT_MQTT_Unsubscribe(pclient, TOPIC_RELAY);
    HAL_SleepMs(200);
    //printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    IOT_MQTT_Destroy(&pclient);

do_exit:
//printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }
    //printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }
    //printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    return rc;
}

void mqtt_proc(void*para)
{
    while(1){   // reconnect to tls
        while(!got_ip_flag){
            vTaskDelay(2000 / portTICK_RATE_MS);
        }
        printf("[ALIYUN] MQTT client example begin, free heap size:%d\n", system_get_free_heap_size());
        mqtt_client();
        printf("[ALIYUN] MQTT client example end, free heap size:%d\n", system_get_free_heap_size());
    }
}

void sntpfn()
{

    os_printf("Initializing SNTP\n");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
//    sntp_setservername(0, "120.25.115.19");
    sntp_setservername(0, "202.112.29.82");        // set sntp server after got ip address, you had better to adjust the sntp server to your area
//    sntp_setservername(1, "time-a.nist.gov");
//    sntp_setservername(2, "ntp.sjtu.edu.cn");
//    sntp_setservername(3, "0.nettime.pool.ntp.org");
//    sntp_setservername(4, "time-b.nist.gov");
//    sntp_setservername(5, "time-a.timefreq.bldrdoc.gov");
//    sntp_setservername(6, "time-b.timefreq.bldrdoc.gov");
//    sntp_setservername(7, "time-c.timefreq.bldrdoc.gov");
//    sntp_setservername(8, "utcnist.colorado.edu");
//    sntp_setservername(9, "time.nist.gov");
//    sntp_setservername(10, "us.pool.ntp.org");
    sntp_init();
    while(1){
        u32_t ts = 0;
        ts = sntp_get_current_timestamp();
        os_printf("current time : %s\n", sntp_get_real_time(ts));
        if (ts == 0) {
            os_printf("did not get a valid time from sntp server\n");
        } else {
            break;
        }
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}

// esp_err_t event_handler(void *ctx, system_event_t *event)
void event_handler(System_Event_t *event)
{
    switch (event->event_id)
    {
    case EVENT_STAMODE_GOT_IP:
        ESP_LOGI(TAG, "Connected.");
        if (system_get_free_heap_size() < 30 * 1024){
            system_restart();
        }
        sntpfn();
        got_ip_flag = 1;
        break;

    case EVENT_STAMODE_DISCONNECTED:
        ESP_LOGI(TAG, "Wifi disconnected, try to connect ...");
        got_ip_flag = 0;
        wifi_station_connect();
        break;

    default:
        break;
    }
}

void initialize_wifi(void)
{
    os_printf("SDK version:%s %d\n", system_get_sdk_version(), system_get_free_heap_size());
     wifi_set_opmode(STATION_MODE);

     // set AP parameter
     struct station_config config;
     bzero(&config, sizeof(struct station_config));
     sprintf(config.ssid, WIFI_SSID);
     sprintf(config.password, WIFI_PASSWORD);
     wifi_station_set_config(&config);

     wifi_station_set_auto_connect(true);
     wifi_station_set_reconnect_policy(true);
     wifi_set_event_handler_cb(event_handler);
}

void heap_check_task(void*para){
    while(1){
        vTaskDelay(2000 / portTICK_RATE_MS);
        printf("[heap check task] free heap size:%d\n", system_get_free_heap_size());
    }
}


void json_test(void) {
    uint8_t json[512] = {0};

    cJSON *root = cJSON_CreateObject();
    cJSON *sensors = cJSON_CreateArray();
    cJSON *id1 = cJSON_CreateObject();
    cJSON *id2 = cJSON_CreateObject();
    cJSON *iNumber = cJSON_CreateNumber(10);

    cJSON_AddItemToObject(id1, "id", cJSON_CreateString("1"));
    cJSON_AddItemToObject(id1, "temperature1", cJSON_CreateString("23"));
    cJSON_AddItemToObject(id1, "temperature2", cJSON_CreateString("23"));
    cJSON_AddItemToObject(id1, "humidity", cJSON_CreateString("55"));
    cJSON_AddItemToObject(id1, "occupancy", cJSON_CreateString("1"));
    cJSON_AddItemToObject(id1, "illumination", cJSON_CreateString("23"));

    cJSON_AddItemToObject(id2, "id", cJSON_CreateString("2"));
    cJSON_AddItemToObject(id2, "temperature1", cJSON_CreateString("23"));
    cJSON_AddItemToObject(id2, "temperature2", cJSON_CreateString("23"));
    cJSON_AddItemToObject(id2, "humidity", cJSON_CreateString("55"));
    cJSON_AddItemToObject(id2, "occupancy", cJSON_CreateString("1"));
    cJSON_AddItemToObject(id2, "illumination", cJSON_CreateString("23"));

    cJSON_AddItemToObject(id2, "value", iNumber);

    cJSON_AddItemToArray(sensors, id1);
    cJSON_AddItemToArray(sensors, id2);

    cJSON_AddItemToObject(root, "sensors", sensors);
#if 0
    char *str = cJSON_Print(root);
#else
    char *str = cJSON_PrintUnformatted(root);
#endif
    uint32_t jslen = strlen(str);
    memcpy(json, str, jslen);
    printf("%s\n", json);

    cJSON_Delete(root);
    free(str);
    str = NULL;
}


void user_init(void)
{
    extern unsigned int max_content_len;
    max_content_len = 4 * 1024;
    printf("SDK version:%s \n", system_get_sdk_version());
    printf("\n******************************************  \n  SDK compile time:%s %s\n******************************************\n\n", __DATE__, __TIME__);
    IOT_OpenLog("mqtt");
    IOT_SetLogLevel(IOT_LOG_DEBUG);
    initialize_wifi();

#if 1
    //xTaskCreate(heap_check_task, "heap_check_task", 128, NULL, 5, NULL);
    xTaskCreate(mqtt_proc, "mqttex", 2048, NULL, 5, NULL);
#endif

// DO JSON Test
#if 0
    json_test();
#endif


}


