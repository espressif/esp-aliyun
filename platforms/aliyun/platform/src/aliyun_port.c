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
#include <stdint.h>
#include "aliyun_port.h"

void print_debug(const char *data, const int len, const char *note)
{
#define COUNT_BYTE_AND_NEW_LINE 0
#define ALL_BINARY_SHOW 0
    os_printf("\n********** %s [len:%d] start addr:%p **********\n", note, len, data);
    int i = 0;

    for (i = 0; i < len; ++i) {
#if !(ALL_BINARY_SHOW)

        if (data[i] < 33 || data[i] > 126) {
            if (i > 0 && (data[i - 1] >= 33 && data[i - 1] <= 126)) {
                os_printf(" ");
            }

            os_printf("%02x ", data[i]);
        } else {
            os_printf("%c", data[i]);
        }

#else
        os_printf("%02x ", data[i]);
#endif

#if COUNT_BYTE_AND_NEW_LINE

        if ((i + 1) % 32 == 0) {
            os_printf("    | %d Bytes\n", i + 1);
        }

#endif
    }

    os_printf("\n---------- %s End ----------\n", note);
}

void print_error(const char *file, const char *function, uint32_t line, const char *note)
{
    while(1){
        os_printf("[error:%s]file:%s function:%s line:%u heap size:%d\n", note, file, function, line, system_get_free_heap_size());
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}
