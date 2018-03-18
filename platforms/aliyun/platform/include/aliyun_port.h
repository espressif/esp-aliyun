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

#ifndef ALIYUN_PORT_H_
#define ALIYUN_PORT_H_

/**
 * @brief  print the data detail information
 *
 * print input data, print from data[0] to data[len-1], addtionally add notes string
 *
 * @param[in]  data: input data pointer to print
 * @param[in]  len: data length
 * @param[in]  note: notes for read easily
 *
 * @return noreturn
 *
 */
void print_debug(const char *data, const int len, const char *note);

/**
 * @brief  print the fatal error information and cycle it
 *
 *  usage: same to printf
 *
 * @return noreturn
 * */

#define print_error(fmt, args...)  \
    do { \
        printf("[error]:");\
        printf(fmt, ##args); \
        printf(",heap size:%d%s", system_get_free_heap_size(), "\r\n"); \
        vTaskDelay(2000 / portTICK_RATE_MS);    \
    } while(1)


#endif
