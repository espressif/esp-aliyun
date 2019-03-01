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
#include <sdkconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#ifdef CONFIG_TARGET_PLATFORM_ESP8266
#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"
#include "driver/pwm.h"
#else
#include "driver/ledc.h"
#endif
#include "esp_log.h"
#include "light_control.h"

// default:
// GPIO4  ->  Red
// GPIO5  ->  Green
// GPIO21 ->  Blue
#define LEDC_IO_0 (4)
#define LEDC_IO_1 (5)
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
#define LEDC_IO_2 (16)
#else
#define LEDC_IO_2 (21)
#endif

#define PWM_DEPTH (1023)

typedef struct hsp {
    uint16_t h;  // 0-360
    uint16_t s;  // 0-100
    uint16_t b;  // 0-100
} hsp_t;

static hsp_t s_hsb_val;
static uint16_t s_brightness;
static bool s_on = false;

static bulb_state_t s_bulb_state = {false, 0, 0, 0, 0};

static const char *TAG = "light bulb";

#ifdef CONFIG_TARGET_PLATFORM_ESP8266
#define PWM_PERIOD    (500)
#define PWM_IO_NUM    3

// pwm pin number
const uint32_t pin_num[PWM_IO_NUM] = {
    LEDC_IO_0,
    LEDC_IO_1,
    LEDC_IO_2
};

// dutys table, (duty/PERIOD)*depth
uint32_t duties[PWM_IO_NUM] = {
    250, 250, 250,
};

// phase table, (phase/180)*depth
int16_t phase[PWM_IO_NUM] = {
    0, 0, 50,
};

#define LEDC_CHANNEL_0 0
#define LEDC_CHANNEL_1 1
#define LEDC_CHANNEL_2 2

#endif

/**
 * @brief transform lightbulb's "RGB" and other parameter
 */
void lightbulb_set_aim(uint32_t r, uint32_t g, uint32_t b, uint32_t cw, uint32_t ww, uint32_t period)
{
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    printf("lightbulb_set_aim r %d g %d b %d\r\n", r, g, b);
    pwm_stop(0x3);
    pwm_set_duty(LEDC_CHANNEL_0, r);
    pwm_set_duty(LEDC_CHANNEL_1, g);
    pwm_set_duty(LEDC_CHANNEL_2, b);
    pwm_start();
#else
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, r);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, g);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, b);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);
#endif
}

/**
 * @brief transform lightbulb's "HSV" to "RGB"
 */
static bool lightbulb_set_hsb2rgb(uint16_t h, uint16_t s, uint16_t v, rgb_t *rgb)
{
    bool res = true;
    uint16_t hi, F, P, Q, T;

    if (!rgb) {
        return false;
    }

    if (h > 360) {
        return false;
    }

    if (s > 100) {
        return false;
    }

    if (v > 100) {
        return false;
    }

    hi = (h / 60) % 6;
    F = 100 * h / 60 - 100 * hi;
    P = v * (100 - s) / 100;
    Q = v * (10000 - F * s) / 10000;
    T = v * (10000 - s * (100 - F)) / 10000;

    switch (hi) {
    case 0:
        rgb->r = v;
        rgb->g = T;
        rgb->b = P;
        break;

    case 1:
        rgb->r = Q;
        rgb->g = v;
        rgb->b = P;
        break;

    case 2:
        rgb->r = P;
        rgb->g = v;
        rgb->b = T;
        break;

    case 3:
        rgb->r = P;
        rgb->g = Q;
        rgb->b = v;
        break;

    case 4:
        rgb->r = T;
        rgb->g = P;
        rgb->b = v;
        break;

    case 5:
        rgb->r = v;
        rgb->g = P;
        rgb->b = Q;
        break;

    default:
        return false;
    }

    return res;
}

/**
 * @brief set the lightbulb's "HSV"
 */
bool lightbulb_set_aim_hsv(uint16_t h, uint16_t s, uint16_t v)
{
    rgb_t rgb_tmp;
    bool ret = lightbulb_set_hsb2rgb(h, s, v, &rgb_tmp);

    if (ret == false) {
        ESP_LOGE(TAG, "lightbulb_set_hsb2rgb failed");
        return false;
    }

    lightbulb_set_aim(rgb_tmp.r * PWM_TARGET_DUTY / 100, rgb_tmp.g * PWM_TARGET_DUTY / 100,
                      rgb_tmp.b * PWM_TARGET_DUTY / 100, (100 - s) * 5000 / 100, v * 2000 / 100, 1000);

    return true;
}

/**
 * @brief update the lightbulb's state
 */
static void lightbulb_update()
{
    lightbulb_set_aim_hsv(s_hsb_val.h, s_hsb_val.s, s_hsb_val.b);
}

/**
 * @brief initialize the lightbulb lowlevel module
 */
void lightbulb_init(void)
{
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    pwm_init(PWM_PERIOD, duties, PWM_IO_NUM, pin_num);
    pwm_set_channel_invert(0x1 << 0);
    pwm_set_phases(phase);
    pwm_start();
#else
    // enable ledc module
    periph_module_enable(PERIPH_LEDC_MODULE);

    // config the timer
    ledc_timer_config_t ledc_timer = {
        //set timer counter bit number
        .bit_num = LEDC_TIMER_13_BIT,
        //set frequency of pwm
        .freq_hz = 5000,
        //timer mode,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        //timer index
        .timer_num = LEDC_TIMER_0
    };
    ledc_timer_config(&ledc_timer);

    //config the channel
    ledc_channel_config_t ledc_channel = {
        //set LEDC channel 0
        .channel = LEDC_CHANNEL_0,
        //set the duty for initialization.(duty range is 0 ~ ((2**bit_num)-1)
        .duty = 100,
        //GPIO number
        .gpio_num = LEDC_IO_0,
        //GPIO INTR TYPE, as an example, we enable fade_end interrupt here.
        .intr_type = LEDC_INTR_FADE_END,
        //set LEDC mode, from ledc_mode_t
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        //set LEDC timer source, if different channel use one timer,
        //the frequency and bit_num of these channels should be the same
        .timer_sel = LEDC_TIMER_0
    };
    //set the configuration
    ledc_channel_config(&ledc_channel);

    //config ledc channel1
    ledc_channel.channel = LEDC_CHANNEL_1;
    ledc_channel.gpio_num = LEDC_IO_1;
    ledc_channel_config(&ledc_channel);
    //config ledc channel2
    ledc_channel.channel = LEDC_CHANNEL_2;
    ledc_channel.gpio_num = LEDC_IO_2;
    ledc_channel_config(&ledc_channel);
#endif
    s_hsb_val.b = 0;
    s_hsb_val.s = 0;
    s_hsb_val.h = 0;
    lightbulb_update();
}

/**
 * @brief deinitialize the lightbulb's lowlevel module
 */
void lightbulb_deinit(void)
{
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    pwm_stop(0x3);
#else
    ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2, 0);
#endif
}

/**
 * @brief turn on/off the lowlevel lightbulb
 */
void lightbulb_set_on(void *p)
{
    bool value = *(bool *)p;

    // ESP_LOGI(TAG, "lightbulb_set_on : %s", value == true ? "true" : "false");

    if (value == true) {
        s_hsb_val.b = s_brightness;
        s_on = true;
    } else {
        s_brightness = s_hsb_val.b;
        s_hsb_val.b = 0;
        s_on = false;
    }

    lightbulb_update();
    return;
}

/**
 * @brief  set light state off
 *
 * @param[in]  no parameter input
 *
 * @return  noreturn
 *
 * */
void lightbulb_set_off()
{
    s_hsb_val.b = 0;
    s_on = false;
    lightbulb_update();
}

/**
 * @brief set the saturation of the lowlevel lightbulb
 */
void lightbulb_set_saturation(void *p)
{
    double value = *(double *)p;

    // ESP_LOGI(TAG, "lightbulb_set_saturation : %f", value);

    s_hsb_val.s = value;

    if (true == s_on) {
        lightbulb_update();
    }

    return;
}

/**
 * @brief set the hue of the lowlevel lightbulb
 */
void lightbulb_set_hue(void *p)
{
    double value = *(double *)p;

    // ESP_LOGI(TAG, "lightbulb_set_hue : %f", value);

    s_hsb_val.h = value;

    if (true == s_on) {
        lightbulb_update();
    }

    return;
}

/**
 * @brief set the brightness of the lowlevel lightbulb
 */
void lightbulb_set_brightness(void *p)
{
    int value = *(int *)p;

    // ESP_LOGI(TAG, "lightbulb_set_brightness : %d", value);

    s_hsb_val.b = value;
    s_brightness = s_hsb_val.b;

    if (true == s_on) {
        lightbulb_update();
    }

    return;
}

/**
 * @brief  notify light state to set, time interval to set
 *
 * @param[in] state state to set
 * @param[in] time interval to set
 *
 * @return  noreturn
 *
 * */
void notify_lightbulb_state(bulb_color_t state, int flash_interval)
{
    switch (state) {
    case BULB_STATE_OFF: {
        s_bulb_state.set_on = false;
        break;
    }

    case BULB_STATE_RED: {
        s_bulb_state.set_on = true;
        s_bulb_state.hue_value = 0;
        s_bulb_state.saturation_value = 100;
        s_bulb_state.brightness_value = 20;
        break;
    }

    case BULB_STATE_GREEN: {
        s_bulb_state.set_on = true;
        s_bulb_state.hue_value = 120;
        s_bulb_state.saturation_value = 100;
        s_bulb_state.brightness_value = 20;
        break;
    }

    case BULB_STATE_BLUE: {
        s_bulb_state.set_on = true;
        s_bulb_state.hue_value = 240;
        s_bulb_state.saturation_value = 100;
        s_bulb_state.brightness_value = 20;
        break;
    }

    case BULB_STATE_OTHERS: {
        s_bulb_state.set_on = true;
        s_bulb_state.hue_value = rand() % 360;
        s_bulb_state.saturation_value = rand() % 50;
        s_bulb_state.brightness_value = rand() % 50;
        break;
    }

    default:
        ESP_LOGI(TAG, "only support RED,GREEN,BLUE,OTHERs COLOR!");
    }

    s_bulb_state.flash_interval = flash_interval;
    ESP_LOGI(TAG, "set on/off state:%d flash interval:%d H:%f S:%f B:%d", \
             s_bulb_state.set_on, s_bulb_state.flash_interval, s_bulb_state.hue_value, s_bulb_state.saturation_value, s_bulb_state.brightness_value);
}

/**
 * @brief get current light state
 *
 * @param[in]  in parameter input
 *
 * @return  struct bulb_state_t which including light state
 * */
bulb_state_t *get_current_bulb_state()
{
    return &s_bulb_state;
}

/**
 * @brief set current light state
 *
 * @param[in]  input_save_bulb_state: struct bulb_state_t which including light state
 *
 * @return  noreturn
 * */
void set_current_bulb_state(bulb_state_t input_save_bulb_state)
{
    s_bulb_state = input_save_bulb_state;
}

// entry
void led_light_start()
{
    lightbulb_init();
}
