/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */




#ifndef __PLATFORM_DEBUG_H__
#define __PLATFORM_DEBUG_H__

#include "esp_log.h"

static const char* TAG = "in";

#define hal_emerg(...)      ESP_LOGE(TAG, __VA_ARGS__)
#define hal_crit(...)       ESP_LOGE(TAG, __VA_ARGS__)
#define hal_err(...)        ESP_LOGE(TAG, __VA_ARGS__)
#define hal_warning(...)    ESP_LOGW(TAG, __VA_ARGS__)
#define hal_info(...)       ESP_LOGI(TAG, __VA_ARGS__)
#define hal_debug(...)      ESP_LOGD(TAG, __VA_ARGS__)

#endif  /* __PLATFORM_DEBUG_H__ */
