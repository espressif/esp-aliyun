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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "app_entry.h"
#include "driver/uart.h"

#include "transport_uart.h"
#include "transport_data.h"

#if defined(CONFIG_IDF_TARGET_ESP32)
#define TRANSPORT_UART_TASK_SIZE          6 * 1024
#define TRANSPORT_UART_BAUD_RATE          115200
#else
#define TRANSPORT_UART_TASK_SIZE          2 * 1024
#define TRANSPORT_UART_BAUD_RATE          115200
#endif
#define TRANSPORT_EX_UART_NUM                          UART_NUM_0

static uint8_t buffer[TRANSPORT_UART_RECV_BUF_SIZE] = {0};

static const char *TAG = "transport_uart";

#if __has_include("esp_idf_version.h")
#include "esp_idf_version.h"
#else
/** Major version number (X.x.x) */
#define ESP_IDF_VERSION_MAJOR   3
/** Minor version number (x.X.x) */
#define ESP_IDF_VERSION_MINOR   2
/** Patch version number (x.x.X) */
#define ESP_IDF_VERSION_PATCH   0
#define ESP_IDF_VERSION_VAL(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#define ESP_IDF_VERSION  ESP_IDF_VERSION_VAL(ESP_IDF_VERSION_MAJOR, \
                                             ESP_IDF_VERSION_MINOR, \
                                             ESP_IDF_VERSION_PATCH)
#endif

static QueueHandle_t s_transport_uart_queue;

/* uart init open */
static int transport_uart_init(void)
{
    // Configure parameters of an UART driver,
    // communication pins and install the driver
    uart_config_t uart_conf = {
        .baud_rate = TRANSPORT_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 120
    };

    uart_param_config(TRANSPORT_EX_UART_NUM, &uart_conf);

#if defined(CONFIG_IDF_TARGET_ESP32)
    //Set UART pins,(-1: default pin, no change.)
    uart_set_pin(TRANSPORT_EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
#endif
    // Install UART driver, and get the queue.
#if CONFIG_IDF_TARGET_ESP8266 && ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(3, 3, 0)
    uart_driver_install(TRANSPORT_EX_UART_NUM, TRANSPORT_UART_RECV_BUF_SIZE, 0, TRANSPORT_UART_QUEUE_SIZE, &s_transport_uart_queue);
#else
    uart_driver_install(TRANSPORT_EX_UART_NUM, TRANSPORT_UART_RECV_BUF_SIZE, 0, TRANSPORT_UART_QUEUE_SIZE, &s_transport_uart_queue, 0);
#endif

    return 1;
}
bool transport_task_exist_flag = false;
static void transport_uart_task(void *pvParameter)
{
    uart_event_t event;
    size_t buffered_size = 0;
    for (;;) {
        if (transport_task_exist_flag) {
            break;
        }
        if (xQueueReceive(s_transport_uart_queue, (void * )&event, 200 / portTICK_PERIOD_MS)) {
            bzero(buffer, TRANSPORT_UART_RECV_BUF_SIZE);
            switch (event.type) {
                // Event of UART receving data
                // We'd better handler data event fast, there would be much more data events than
                // other types of events. If we take too much time on data event, the queue might be full.
                case UART_DATA:
                    buffered_size = uart_read_bytes(TRANSPORT_EX_UART_NUM, buffer, event.size, portMAX_DELAY);
                    app_get_input_param((char *)buffer, buffered_size);
                    break;
                // Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(TRANSPORT_EX_UART_NUM);
                    xQueueReset(s_transport_uart_queue);
                    break;

                // Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(TRANSPORT_EX_UART_NUM);
                    xQueueReset(s_transport_uart_queue);
                    break;

                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;

                // Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;

                // Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }

    vTaskDelete(NULL);
}

void transport_uart_task_create(void)
{
    xTaskCreate(&transport_uart_task, "transport_uart_task", TRANSPORT_UART_TASK_SIZE, NULL, 4, NULL);
}

int transport_uart_handle_init(void) 
{
    transport_uart_init();
    
    transport_uart_task_create();

    return 0;
}

