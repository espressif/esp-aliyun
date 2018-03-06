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

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "iot_export.h"
#include "aliyun_config.h"
#include "mqtt.h"

extern int got_ip_flag;

// MQTT callback function
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
        print_debug(topic_info->ptopic, topic_info->topic_len, "event topic");
        print_debug(topic_info->payload, topic_info->payload_len, "event payload");
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
    print_debug(ptopic_info->ptopic, ptopic_info->topic_len, "arrive topic");
    print_debug(ptopic_info->payload, ptopic_info->payload_len, "arrive payload");
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

    if (NULL == (msg_buf = (char *)HAL_Malloc(MSG_LEN_MAX))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(MSG_LEN_MAX))) {
        EXAMPLE_TRACE("not enough memory");
        rc = -1;
        goto do_exit;
    }

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, (void **)&pconn_info)) {
        EXAMPLE_TRACE("AUTH request failed!");
        rc = -1;
        goto do_exit;
    }

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
    pclient = IOT_MQTT_Construct(&mqtt_params);

    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        rc = -1;
        goto do_exit;
    }

    /* Subscribe the specific topic */
    rc = IOT_MQTT_Subscribe(pclient, TOPIC_DATA, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);

    if (rc < 0) {
        IOT_MQTT_Destroy(&pclient);
        EXAMPLE_TRACE("IOT_MQTT_Subscribe() failed, rc = %d", rc);
        rc = -1;
        goto do_exit;
    }

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
        if (got_ip_flag) {
            msg_len = snprintf(msg_pub, sizeof(msg_pub), "{\"attr_name\":\"temperature\", \"attr_value\":\"%d\"}", cnt);

            if (msg_len < 0) {
                EXAMPLE_TRACE("Error occur! Exit program");
                rc = -1;
                break;
            }

            topic_msg.payload = (void *)msg_pub;
            topic_msg.payload_len = msg_len;

            rc = IOT_MQTT_Publish(pclient, TOPIC_DATA, &topic_msg);

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
        } else {

            break;
        }

        /* Generate topic message */
    }

    IOT_MQTT_Unsubscribe(pclient, TOPIC_DATA);
    IOT_MQTT_Unsubscribe(pclient, TOPIC_RELAY);

    IOT_MQTT_Destroy(&pclient);

do_exit:

    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    return rc;
}


void mqtt_proc(void *pvParameter)
{
    while (1) { // reconnect to tls
        while (!got_ip_flag) {
            vTaskDelay(TASK_CYCLE / portTICK_RATE_MS);
        }

        os_printf("[ALIYUN] MQTT client example begin, free heap size:%d\n", system_get_free_heap_size());
        mqtt_client();
        os_printf("[ALIYUN] MQTT client example end, free heap size:%d\n", system_get_free_heap_size());
    }
}
