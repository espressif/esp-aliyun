/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP32 only, in which case,
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
#ifndef LIGHT_CONTROL_H_
#define LIGHT_CONTROL_H_
#include <stdbool.h>

#define PWM_TARGET_DUTY 8192

typedef enum {
    BULB_STATE_OFF = 0,
    BULB_STATE_RED = 1,
    BULB_STATE_GREEN = 2,
    BULB_STATE_BLUE = 3,
    BULB_STATE_OTHERS = 4
} bulb_color_t;

typedef struct bulb_state {
    bool set_on;
    double hue_value;
    double saturation_value;
    int brightness_value;
    int flash_interval;
} bulb_state_t;

typedef struct {
    uint8_t r;  // 0-100 %
    uint8_t g;  // 0-100 %
    uint8_t b;  // 0-100 %
} rgb_t;

/**
 * @brief initialize the lightbulb lowlevel module
 *
 * @param none
 *
 * @return none
 */
void lightbulb_init(void);

/**
 * @brief deinitialize the lightbulb's lowlevel module
 *
 * @param none
 *
 * @return none
 */
void lightbulb_deinit(void);

/**
 * @brief turn on/off the lowlevel lightbulb
 *
 * @param p signal point, the data is type of "bool"
 *
 * @return none
 */
void lightbulb_set_on(void *p);

/**
 * @brief set the saturation of the lowlevel lightbulb
 *
 * @param p signal point, the data is type of "int"
 *
 * @return
 *     - 0 : OK
 *     - others : fail
 */
void lightbulb_set_saturation(void *p);

/**
 * @brief set the hue of the lowlevel lightbulb
 *
 * @param p signal point, the data is type of "double"
 *
 * @return
 *     - 0 : OK
 *     - others : fail
 */
void lightbulb_set_hue(void *p);

/**
 * @brief set the brightness of the lowlevel lightbulb
 *
 * @param p signal point, the data is type of "double"
 *
 * @return
 *     - 0 : OK
 *     - others : fail
 */
void lightbulb_set_brightness(void *p);

/**
 * @brief  notify light state to set, time interval to set
 *
 * @param[in] state state to set
 * @param[in] time interval to set
 *
 * @return  noreturn
 *
 * */
void notify_lightbulb_state(bulb_color_t in_color, int flash_interval);

/**
 * @brief get current light state
 *
 * @param[in]  in parameter input
 *
 * @return  struct bulb_state_t which including light state
 * */
bulb_state_t *get_current_bulb_state();

/**
 * @brief set current light state
 *
 * @param[in]  input_save_bulb_state: struct bulb_state_t which including light state
 *
 * @return  noreturn
 * */
void set_current_bulb_state(bulb_state_t input_save_bulb_state);

/**
 * @brief  set light state off
 *
 * @param[in]  no parameter input
 *
 * @return  noreturn
 *
 * */
void lightbulb_set_off();

// main light state damon entry
void led_light_start();

// for system state
void notify_lightbulb_state(bulb_color_t state, int flash_interval);

// for hsv
bool lightbulb_set_aim_hsv(uint16_t h, uint16_t s, uint16_t v);

void lightbulb_set_aim(uint32_t r, uint32_t g, uint32_t b, uint32_t cw, uint32_t ww, uint32_t period);

#endif /* LIGHTBULB_H_ */

