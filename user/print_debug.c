/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
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

#include "print_debug.h"

/**
 * @brief  print the data detail information
 *
 * print input data, print from data[0] to data[len-1], addtionally add notes string
 *
 * @param[in]  data: input data pointer to print
 * @param[in]  len: data length
 * @param[in]  note: notes for read easily
 * @param[in]  mode: 0x00, 0x01, 0x10, 0x11 to decide the BINARY_SHOW && BYTES_SHOW
 *
 * @return noreturn
 *
 */
void print_debug(const char* data, const unsigned int len, const char* note, int mode)
{
#define BINARY_SHOW 0x10
#define BYTES_SHOW  0x01
    printf("\n********** %s [len:%u] start addr:%p **********\n", note, len, data);
    int i = 0;
    for (i = 0; i < len; ++i) {
        if (BINARY_SHOW & mode) {
            printf("%02x ",data[i]);
        } else {
            if(data[i] < 32 || data[i] > 126) { // control || invisible charset
                if(i > 0 && (data[i-1] >= 33 && data[i-1] <= 126) )
                        printf(" ");
                printf("%02x ",data[i]);
             } else {
                printf("%c", data[i]);
             }
        }

        if ((BYTES_SHOW & mode) && ((i + 1) % 32 == 0)) {
                printf("    | %d Bytes\n",i + 1);
        }
    }   // end for

    printf("\n---------- %s End ----------\n", note);
}
