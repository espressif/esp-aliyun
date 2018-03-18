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

#include "iot_export_ota.h"

#include "esp_common.h"
#include "lwip/mem.h"
#include "aliyun_config.h"
#include "ota.h"
#include "aliyun_port.h"

extern bool http_200_check;
extern bool resp_body_start;
extern uint32 download_length;
extern int got_ip_flag;
extern ota_info_t *p_ota_info;
extern os_timer_t upgrade_timer;


extern int ota_report_latest_version;


/******************************************************************************
 * FunctionName : ota_begin
 * Description  : ota_task function
 * Parameters   : task param
 * Returns      : none
*******************************************************************************/
static void remote_ota_begin()
{
    int read_bytes;
    int sin_size;
    char recv_buf[1460];
    int sta_socket;
    struct sockaddr_in remote_ip;
    printf("Hello, welcome to remote ota!\r\n");

    while (1) {
        sta_socket = socket(PF_INET, SOCK_STREAM, 0);

        if (-1 == sta_socket) {
            close(sta_socket);
            printf("socket fail !\r\n");
            continue;
        }

        printf("socket ok!\r\n");
        bzero(&remote_ip, sizeof(struct sockaddr_in));
        remote_ip.sin_family = AF_INET;

        remote_ip.sin_addr.s_addr = (p_ota_info->target_ip).addr;
        remote_ip.sin_port = htons(REMOTE_OTA_SERVER_PORT);

        if (0 != connect(sta_socket, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr))) {
            close(sta_socket);
            printf("connect fail!\r\n");
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            upgrade_recycle();
        }

        printf("connect ok!\r\n");

        /*send GET request to http server*/
        const char *REMOTE_GET_FORMAT =
            "GET %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "User-Agent: esp8266-rtos-sdk/1.0 esp8266\r\n\r\n";

        char *http_request = NULL;
        int get_len = 0;
        get_len = asprintf(&http_request, REMOTE_GET_FORMAT, (const char *)p_ota_info->ota_path, p_ota_info->hostname);

        if (get_len < 0) {
            printf("malloc memory failed.\n");
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            upgrade_recycle();
        }

        // send http request
        if (write(sta_socket, http_request, strlen(http_request) + 1) < 0) {
            close(sta_socket);
            printf("send fail\n");
            free(http_request);
            upgrade_recycle();
        }

        printf("send success\n");
        free(http_request);

        while ((read_bytes = read(sta_socket , recv_buf, 1460)) >= 0) {
            if (read_bytes > 0) {
                upgrade_download(sta_socket, recv_buf, read_bytes);
            } else {
                printf("peer close socket\n");
                break;
            }

            // default bin size equals to p_ota_info->bin_size
            if (download_length == p_ota_info->bin_size && download_length != 0) {
                printf("upgrade file download finished, bin size:%d\n", download_length);

                if (upgrade_crc_check(system_get_fw_start_sec(), download_length) != true) {
                    printf("upgrade crc check failed !\n");
                    system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
                } else {
                    printf("bin check crc ok\n");
                    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
                }

                upgrade_recycle();
            }
        }

        printf("read_bytes = %d\n", read_bytes);

        if (read_bytes < 0) {
            printf("read data fail!\r\n");
            close(sta_socket);
            system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            upgrade_recycle();
        }
    }
}

// remote_ota_upgrade_task
void remote_ota_upgrade_task(void *pvParameter)
{
    printf("\nremote OTA task started...\n");

    while (!got_ip_flag) {
        vTaskDelay(TASK_CYCLE / portTICK_RATE_MS);
        printf("wait for fetching IP...\n");
    }

    printf("ota begin, free heap size:%d\n", system_get_free_heap_size());
    system_upgrade_flag_set(UPGRADE_FLAG_START);
    system_upgrade_init();

    remote_ota_begin();

    // OTA timeout
    os_timer_disarm(&upgrade_timer);
    os_timer_setfn(&upgrade_timer, (os_timer_func_t *)upgrade_recycle, NULL);
    os_timer_arm(&upgrade_timer, OTA_TIMEOUT, 0);
}
