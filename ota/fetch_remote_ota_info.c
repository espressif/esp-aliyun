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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "iot_export.h"
#include "aliyun_config.h"
#include "ota.h"
#include "mqtt.h"
#include "cJSON.h"
#include "time.h"
#include "aliyun_port.h"

extern int got_ip_flag;
extern ota_info_t *p_ota_info;

int ota_process = 1;
int ota_report_latest_version = 0;
char msg_pub[MSG_PUB_MAX_SIZE] = { 0 };
int msg_len = 0;

// read buffer by byte still delimiter appear total_count times
// return delimiter current position in buffer
// return < 0 means could not find delim for total_count times
static int read_until_counts(char *buffer, char delim, int len, int total_count)
{
    /*TODO: buffer check, delim check, count check,further: do an buffer length limited*/
    int i = 0, valid_count = 0;

    for (i = 0; i < len; ++i) {
        if (buffer[i] == delim) {
            valid_count++;
        }

        if (valid_count == total_count) {
            return i;
        }
    }

    return -1;
}

// callback of upgrade topic
void upgrade_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_pt ptopic_info = (iotx_mqtt_topic_info_pt) msg->msg;
    cJSON *root, *data_item, *size_item, *version_item, *url_item;
    int ret = 0;
    ip_addr_t target_ip;

    // print topic name and topic message
    print_debug(ptopic_info->ptopic, ptopic_info->topic_len, "upgrade topic");
    print_debug(ptopic_info->payload, ptopic_info->payload_len, "upgrade payload");

    // parse payload which is json format, save it to flash
    // ota json format should obey https://help.aliyun.com/document_detail/58106.html?spm=a2c4g.11174283.6.569.JBBgOw
    root = cJSON_Parse((char *)ptopic_info->payload);

    if (!root) {
        print_error("Error before: [%s]", cJSON_GetErrorPtr());
    }

    data_item = cJSON_GetObjectItem(root, "data");

    if (!data_item) {
        print_error("get data item");
    }

    size_item = cJSON_GetObjectItem(data_item, "size");

    if (!size_item) {
        print_error("get size item");
    }

    version_item = cJSON_GetObjectItem(data_item, "version");

    if (!version_item) {
        print_error("get version item");
    }

    url_item = cJSON_GetObjectItem(data_item, "url");

    if (!url_item) {
        print_error("get url item");
    }

    char *p = url_item->valuestring;
    // p pointer point at the beginning of whole URL string
    // fetch domain name for URL by delimiter '/'
    int start_pos = read_until_counts(p, '/', strlen(p), 2);
    int end_pos = read_until_counts(p, '/', strlen(p), 3);
    char *target_name = (char *)zalloc(end_pos - start_pos);
    memcpy(target_name, p + start_pos + 1, end_pos - start_pos - 1);

    // get ip by hostname
    do {
        ret = netconn_gethostbyname(target_name, &target_ip);
    } while (ret);

    // save ota information to flash, load it after system restart
    memset(p_ota_info, 0, sizeof(ota_info_t));
    p_ota_info->ota_flag = 1;
    p_ota_info->target_ip = target_ip;
    p_ota_info->port = REMOTE_OTA_SERVER_PORT;
    // notice that bin size JSON type is cJSON_Object rather than cJSON_Number
    p_ota_info->bin_size = data_item->child->valueint;
    memcpy(p_ota_info->latest_version, version_item->valuestring, strlen(version_item->valuestring));
    memcpy(p_ota_info->hostname, target_name, strlen(target_name));
    memcpy(p_ota_info->ota_path, p + end_pos, strlen(p) - end_pos);

    if (system_param_save_with_protect(PARAM_SAVE_SEC, p_ota_info, sizeof(ota_info_t)) != true) {
        print_error("para save");
    }

    // inform aliyun console ota process
    ota_process = 100;
    free(target_name);
}

// construct report version topic message in json format
void construct_report_version_info(iotx_mqtt_topic_info_t *topic_msg, const char *version_info)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *version = cJSON_CreateObject();
    cJSON *iNumber = cJSON_CreateNumber(1); // reserved, message ID
    cJSON_AddItemToObject(root, "id", iNumber);
    cJSON_AddItemToObject(version, "version", cJSON_CreateString((const char *)version_info));
    cJSON_AddItemToObject(root, "params", version);

    char *str = cJSON_Print(root);
    msg_len = strlen(str);
    memset(msg_pub, 0, MSG_PUB_MAX_SIZE);

    // copy json to publish buffer
    memcpy(msg_pub, str, msg_len);
    topic_msg->payload = (void *)msg_pub;
    topic_msg->payload_len = msg_len;

    cJSON_Delete(root);
    free(str);
    str = NULL;
}

// construct report process topic message in json format
void construct_report_process_info(iotx_mqtt_topic_info_t *topic_msg)
{
    memset(topic_msg, 0x0, sizeof(iotx_mqtt_topic_info_t));
    cJSON *root = cJSON_CreateObject();
    cJSON *params = cJSON_CreateObject();
    cJSON *iNumber = cJSON_CreateNumber(1); // reserved, message ID
    cJSON_AddItemToObject(root, "id", iNumber);

    char c[3] = {0};
    sprintf(c, "%d", ota_process);
    cJSON_AddItemToObject(params, "step", cJSON_CreateString((const char *)c));
    cJSON_AddItemToObject(params, "desc", cJSON_CreateString("process"));

    cJSON_AddItemToObject(root, "params", params);
    char *str = cJSON_Print(root);
    msg_len = strlen(str);
    memset(msg_pub, 0, MSG_PUB_MAX_SIZE);
    memcpy(msg_pub, str, msg_len);
    topic_msg->payload = (void *)msg_pub;
    topic_msg->payload_len = msg_len;

    cJSON_Delete(root);
    free(str);
    str = NULL;
}

int ota_fetch_url_save(void)
{
    int rc = 0, cnt = 0;
    void *pclient = NULL;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    iotx_mqtt_topic_info_t topic_msg;

    char *msg_writebuf = NULL, *msg_readbuf = NULL;

    if (NULL == (msg_writebuf = (char *)HAL_Malloc(MSG_LEN_MAX))) {
        print_error("not enough memory");
    }

    if (NULL == (msg_readbuf = (char *)HAL_Malloc(MSG_LEN_MAX))) {
        print_error("not enough memory");
    }

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET, (void **)&pconn_info)) {
        print_error("AUTH request failed!");
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
    mqtt_params.pwrite_buf = msg_writebuf;
    mqtt_params.write_buf_size = MSG_LEN_MAX;

    mqtt_params.handle_event.h_fp = event_handle;
    mqtt_params.handle_event.pcontext = NULL;

    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);

    if (NULL == pclient) {
        print_error("MQTT construct failed");
    }

    printf("MQTT Connect OK!\n");

    /* Subscribe the upgrade topic */
    rc = IOT_MQTT_Subscribe(pclient, TOPIC_UPGRADE, IOTX_MQTT_QOS1, upgrade_message_arrive, NULL);

    if (rc < 0) {
        print_error("IOT_MQTT_Subscribe TOPIC_UPGRADE failed, rc = %d", rc);
    }

    EXAMPLE_TRACE("packet-id=%d, subscribe upgrade topic", rc);

    // choose current time as current version and report
    char *version_info = NULL;
    time_t t = time(NULL);  // get current time
    struct tm *d = localtime(&t);
    int get_len = asprintf(&version_info, "ESP8266_%04d%02d%02d%02d%02d%02d", d->tm_year + 1900, d->tm_mon + 1, d->tm_mday, d->tm_hour, d->tm_min, d->tm_sec);

    if (get_len < 0) {
        print_error("asprintf ret:%d", get_len);
    }

    // construct report version info and set to topic_msg
    construct_report_version_info(&topic_msg, (const char *)version_info);
    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;

    rc = IOT_MQTT_Publish(pclient, TOPIC_INFORM, &topic_msg);
    free(version_info);

    if (rc < 0) {
        print_error("publish error, rc:%d", rc);
    }

    EXAMPLE_TRACE("packet-id=%d, publish inform topic, msg=%s", rc, msg_pub);

    // report ota process
    while (ota_process < 90) {
        // construct current ota rate of process and report
        construct_report_process_info(&topic_msg);
        topic_msg.qos = IOTX_MQTT_QOS0;
        topic_msg.retain = 0;
        topic_msg.dup = 0;

        rc = IOT_MQTT_Publish(pclient, TOPIC_PROGRESS, &topic_msg);

        if (rc < 0) {
            print_error("error occur when publish,rc:%d", rc);
        }

        EXAMPLE_TRACE("packet-id=%d, publish process topic, msg=%s", rc, msg_pub);

        ota_process += 2;
        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);
        HAL_SleepMs(2000);
    }

    if (ota_process < 100) {
        print_error("cannot get url from aliyun in 90s");
    }

    printf("ready to report latest version.\n");
    // construct topic content in json
    construct_report_version_info(&topic_msg, (const char *)p_ota_info->latest_version);
    topic_msg.qos = IOTX_MQTT_QOS1;
    topic_msg.retain = 0;
    topic_msg.dup = 0;

    rc = IOT_MQTT_Publish(pclient, TOPIC_INFORM, &topic_msg);

    if (rc < 0) {
        print_error("error occur when publish");
    }

    EXAMPLE_TRACE("packet-id=%d, publish topic msg=%s", rc, msg_pub);

    IOT_MQTT_Unsubscribe(pclient, TOPIC_UPGRADE);
    IOT_MQTT_Destroy(&pclient);

    if (NULL != msg_writebuf) {
        HAL_Free(msg_writebuf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    printf("ota info is ready, prepare to restart system\n");
    system_restart();
    return rc;
}

void remote_ota_task(void *para)
{
    while (1) { // reconnect to tls
        while (!got_ip_flag) {
            vTaskDelay(TASK_CYCLE / portTICK_RATE_MS);
        }

        printf("ota_fetch_url begin, free heap size:%d\n", system_get_free_heap_size());
        ota_fetch_url_save();
        printf("ota_fetch_url end, free heap size:%d\n", system_get_free_heap_size());
    }
}
