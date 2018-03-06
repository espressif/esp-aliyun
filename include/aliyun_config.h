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

#ifndef ALIYUN_CONFIG_H_
#define ALIYUN_CONFIG_H_

#define HEAP_CHECK_TASK 0   // check free heap size
#define TASK_CYCLE 2000     // task cycle: 2000 ms

typedef enum {
    MQTT_TASK,          // subscribe & publish messages to aliyun cloud by MQTT
    LOCAL_OTA_TASK,     // local OTA by http
    REMOTE_OTA_TASK,    // TODO: remote OTA by mqtt-tls + http
} TASK_MODE;


/*IMPORTANT: the following configuration maybe need to be modified*/
/***********************************************************************************************************************/

#define DEFAULT_TASK_MODE MQTT_TASK     // there are there modes for user as the enum TASK_MODE depicted, default: mqtt task

// device name, device secret, product key which defined in aliyun cloud
#define PRODUCT_KEY             "********"  // type:string
#define DEVICE_NAME             "********"  // type:string
#define DEVICE_SECRET           "********"  // type:string

// these are pre-defined topics, which interact with aliyun cloud
#define TOPIC_UPDATE            "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
#define TOPIC_ERROR             "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET               "/"PRODUCT_KEY"/"DEVICE_NAME"/get"
#define TOPIC_DATA              "/"PRODUCT_KEY"/"DEVICE_NAME"/data"
#define TOPIC_RELAY             "/"PRODUCT_KEY"/"DEVICE_NAME"/relay"

#define WIFI_SSID       "********"       // type:string, your AP/router SSID to config your device networking
#define WIFI_PASSWORD   "********"       // type:string, your AP/router password

#define MSG_LEN_MAX             (2048)      // max read & write buffer
/***********************************************************************************************************************/

/*Please Keep the following configuration if you have no very deep understanding of ESP8266-aliyun */

extern int got_ip_flag;   // 0: without ip , 1: got ip

#if 1
#define EXAMPLE_TRACE(fmt, args...)  \
    do { \
        os_printf("%s|%d ", __func__, __LINE__); \
        os_printf(fmt, ##args); \
        os_printf("%s", "\r\n"); \
    } while(0)
#else
#define EXAMPLE_TRACE(fmt, args...)  \
    os_printf(fmt, ##args)
#endif

#endif
