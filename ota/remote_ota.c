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

LOCAL os_timer_t ota_timer;
static int binary_file_length = 0;

extern int got_ip_flag;

int ota_client(void)
{
    int rc = 0, ota_over = 0;
    void *pclient = NULL, *h_ota = NULL;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    char *msg_buf = NULL, *msg_readbuf = NULL;
    char buf_ota[OTA_BUF_LEN];

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

    mqtt_params.request_timeout_ms = 2000;
    mqtt_params.clean_session = 0;
    mqtt_params.keepalive_interval_ms = 60000;
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

    h_ota = IOT_OTA_Init(PRODUCT_KEY, DEVICE_NAME, pclient);

    if (NULL == h_ota) {
        rc = -1;
        EXAMPLE_TRACE("initialize OTA failed");
        goto do_exit;
    }

    if (0 != IOT_OTA_ReportVersion(h_ota, "iotx_esp_1.0.0")) {
        rc = -1;
        EXAMPLE_TRACE("report OTA version failed");
        goto do_exit;
    }

    HAL_SleepMs(1000);

    system_upgrade_flag_set(UPGRADE_FLAG_START);
    system_upgrade_init();

    system_upgrade("erase flash", ERASE_FLASH_SIZE);

    // OTA timeout
    os_timer_disarm(&ota_timer);
    os_timer_setfn(&ota_timer, (os_timer_func_t *)upgrade_recycle, NULL);
    os_timer_arm(&ota_timer, OTA_TIMEOUT, 0);

    while (1) {
        if (!ota_over) {
            uint32_t firmware_valid;

            EXAMPLE_TRACE("wait ota upgrade command....");

            /* handle the MQTT packet received from TCP or SSL connection */
            IOT_MQTT_Yield(pclient, 200);

            if (IOT_OTA_IsFetching(h_ota)) {
                uint32_t last_percent = 0, percent = 0;
                char version[128], md5sum[33];
                uint32_t len, size_downloaded, size_file;

                do {
                    len = IOT_OTA_FetchYield(h_ota, buf_ota, OTA_BUF_LEN, 1);

                    if (len > 0) {

                        if (false == system_upgrade(buf_ota, len)) {
                            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                            upgrade_recycle();
                        }

                        binary_file_length += len;
                        os_printf("Have written image length %d\n", binary_file_length);

                    }

                    /* get OTA information */
                    IOT_OTA_Ioctl(h_ota, IOT_OTAG_FETCHED_SIZE, &size_downloaded, 4);
                    IOT_OTA_Ioctl(h_ota, IOT_OTAG_FILE_SIZE, &size_file, 4);
                    IOT_OTA_Ioctl(h_ota, IOT_OTAG_MD5SUM, md5sum, 33);
                    IOT_OTA_Ioctl(h_ota, IOT_OTAG_VERSION, version, 128);

                    last_percent = percent;
                    percent = (size_downloaded * 100) / size_file;

                    if (percent - last_percent > 0) {
                        IOT_OTA_ReportProgress(h_ota, percent, NULL);
                        IOT_OTA_ReportProgress(h_ota, percent, "hello");
                    }

                    IOT_MQTT_Yield(pclient, 100);
                } while (!IOT_OTA_IsFetchFinish(h_ota));

                IOT_OTA_Ioctl(h_ota, IOT_OTAG_CHECK_FIRMWARE, &firmware_valid, 4);

                if (0 == firmware_valid) {
                    EXAMPLE_TRACE("The firmware is invalid");
                } else {
                    if (upgrade_crc_check(system_get_fw_start_sec(), binary_file_length) != true) {
                        os_printf("upgrade crc check failed !\n");
                        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                        upgrade_recycle();
                    }

                    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
                    upgrade_recycle();
                }

                ota_over = 1;
            }

            HAL_SleepMs(2000);
        }
    }

    HAL_SleepMs(200);

do_exit:

    if (NULL != h_ota) {
        IOT_OTA_Deinit(h_ota);
    }

    if (NULL != pclient) {
        IOT_MQTT_Destroy(&pclient);
    }

    if (NULL != msg_buf) {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf) {
        HAL_Free(msg_readbuf);
    }

    return rc;
}

void remote_ota_proc(void *para)
{
    while (1) { // reconnect to tls
        while (!got_ip_flag) {
            vTaskDelay(TASK_CYCLE / portTICK_RATE_MS);
        }

        os_printf("[ALIYUN] OTA client example begin, free heap size:%d\n", system_get_free_heap_size());
        ota_client();
        os_printf("[ALIYUN] OTA client example end, free heap size:%d\n", system_get_free_heap_size());
    }
}
