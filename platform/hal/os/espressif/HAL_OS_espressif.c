/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */





#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// #include <time.h>
// #include <sys/time.h>
// #include <unistd.h>  /* _POSIX_TIMERS */


#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "iot_import.h"
#include "iotx_hal_internal.h"

#include "esp_wifi.h"
#include "esp_timer.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "sdkconfig.h"
#include "platform_hal.h"

static const char* OS_TAG = "HAL-OS";

#define __DEMO__

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 0
#endif

#ifndef sem_t
typedef SemaphoreHandle_t sem_t;
#endif

#ifdef __DEMO__
    char _product_key[PRODUCT_KEY_LEN + 1];
    char _product_secret[PRODUCT_SECRET_LEN + 1];
    char _device_name[DEVICE_NAME_LEN + 1];
    char _device_secret[DEVICE_SECRET_LEN + 1];
#endif

void *HAL_MutexCreate(void)
{
    int err_num;
    pthread_mutex_t *mutex = (pthread_mutex_t *)HAL_Malloc(sizeof(pthread_mutex_t));
    if (NULL == mutex) {
        return NULL;
    }

    if (0 != (err_num = pthread_mutex_init(mutex, NULL))) {
        hal_err("create mutex failed");
        HAL_Free(mutex);
        return NULL;
    }

    return mutex;
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
    int err_num;

    if (!mutex) {
        hal_warning("mutex want to destroy is NULL!");
        return;
    }
    if (0 != (err_num = pthread_mutex_destroy((pthread_mutex_t *)mutex))) {
        hal_err("destroy mutex failed");
    }

    HAL_Free(mutex);
}

void HAL_MutexLock(_IN_ void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_lock((pthread_mutex_t *)mutex))) {
        hal_err("lock mutex failed: - '%s' (%d)", strerror(err_num), err_num);
    }
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_unlock((pthread_mutex_t *)mutex))) {
        hal_err("unlock mutex failed - '%s' (%d)", strerror(err_num), err_num);
    }
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    return malloc(size);
}

void *HAL_Realloc(_IN_ void *ptr, _IN_ uint32_t size)
{
    return realloc(ptr, size);
}

void *HAL_Calloc(_IN_ uint32_t nmemb, _IN_ uint32_t size)
{
    return calloc(nmemb, size);
}

void HAL_Free(_IN_ void *ptr)
{
    free(ptr);
}

#ifdef CONFIG_TARGET_PLATFORM_ESP8266
// notes: clock_gettime has already defined on ESP-IDF
int clock_gettime(int clk_id, struct timespec *t)
{
    struct timeval now;
    int rv = gettimeofday(&now, NULL);
    if (rv) {
        return rv;
    }
    t->tv_sec  = now.tv_sec;
    t->tv_nsec = now.tv_usec * 1000;
    return 0;
}
#endif

#ifdef __APPLE__
uint64_t HAL_UptimeMs(void)
{
    struct timeval tv = { 0 };
    uint64_t time_ms;

    gettimeofday(&tv, NULL);

    time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return time_ms;
}
#else
uint64_t HAL_UptimeMs(void)
{
    uint64_t            time_ms;
    struct timespec     ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    time_ms = ((uint64_t)ts.tv_sec * (uint64_t)1000) + (ts.tv_nsec / 1000 / 1000);

    return time_ms;
}

char *HAL_GetTimeStr(_IN_ char *buf, _IN_ int len)
{
    struct timeval tv;
    struct tm      tm;
    int str_len    = 0;

    if (buf == NULL || len < 28) {
        return NULL;
    }
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tm);
    strftime(buf, 28, "%m-%d %H:%M:%S", &tm);
    str_len = strlen(buf);
    if (str_len + 3 < len) {
        snprintf(buf + str_len, len, ".%3.3d", (int)(tv.tv_usec) / 1000);
    }
    return buf;
}
#endif

void HAL_SleepMs(_IN_ uint32_t ms)
{
    usleep(1000 * ms);
}

void HAL_Srandom(uint32_t seed)
{
    // espressif does not need a seed for esp_random()
#if 0
    srandom(seed);
#endif
}

uint32_t random(void)
{
    return esp_random();
}

uint32_t HAL_Random(uint32_t region)
{
    return (region > 0) ? (random() % region) : 0;
}

int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *format, va_list ap)
{
    return vsnprintf(str, len, format, ap);
}

void HAL_Printf(_IN_ const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}

int HAL_GetPartnerID(char *pid_str)
{
    memset(pid_str, 0x0, PID_STRLEN_MAX);
#ifdef __DEMO__
    strcpy(pid_str, "espressif");
#endif
    return strlen(pid_str);
}

int HAL_GetModuleID(char *mid_str)
{
    memset(mid_str, 0x0, MID_STRLEN_MAX);
#ifdef __DEMO__
    strcpy(mid_str, "wroom");
#endif
    return strlen(mid_str);
}


char *HAL_GetChipID(_OU_ char *cid_str)
{
    memset(cid_str, 0x0, HAL_CID_LEN);
#ifdef __DEMO__
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    strncpy(cid_str, "esp8266", HAL_CID_LEN);
#else
    strncpy(cid_str, "esp32", HAL_CID_LEN);
#endif
    cid_str[HAL_CID_LEN - 1] = '\0';
#endif
    return cid_str;
}


int HAL_GetDeviceID(_OU_ char *device_id)
{
    memset(device_id, 0x0, DEVICE_ID_LEN);
#ifdef __DEMO__
    HAL_Snprintf(device_id, DEVICE_ID_LEN, "%s.%s", _product_key, _device_name);
    device_id[DEVICE_ID_LEN - 1] = '\0';
#endif

    return strlen(device_id);
}

int HAL_SetProductKey(_IN_ char *product_key)
{
    int len = strlen(product_key);
#ifdef __DEMO__
    if (len > PRODUCT_KEY_LEN) {
        return -1;
    }
    memset(_product_key, 0x0, PRODUCT_KEY_LEN + 1);
    strncpy(_product_key, product_key, len);
#endif
    return len;
}


int HAL_SetDeviceName(_IN_ char *device_name)
{
    int len = strlen(device_name);
#ifdef __DEMO__
    if (len > DEVICE_NAME_LEN) {
        return -1;
    }
    memset(_device_name, 0x0, DEVICE_NAME_LEN + 1);
    strncpy(_device_name, device_name, len);
#endif
    return len;
}


int HAL_SetDeviceSecret(_IN_ char *device_secret)
{
    int len = strlen(device_secret);
#ifdef __DEMO__
    if (len > DEVICE_SECRET_LEN) {
        return -1;
    }
    memset(_device_secret, 0x0, DEVICE_SECRET_LEN + 1);
    strncpy(_device_secret, device_secret, len);
#endif
    return len;
}


int HAL_SetProductSecret(_IN_ char *product_secret)
{
    int len = strlen(product_secret);
#ifdef __DEMO__
    if (len > PRODUCT_SECRET_LEN) {
        return -1;
    }
    memset(_product_secret, 0x0, PRODUCT_SECRET_LEN + 1);
    strncpy(_product_secret, product_secret, len);
#endif
    return len;
}

int HAL_GetProductKey(_OU_ char *product_key)
{
    int len = strlen(_product_key);
    memset(product_key, 0x0, PRODUCT_KEY_LEN);

#ifdef __DEMO__
    strncpy(product_key, _product_key, len);
#endif

    return len;
}

int HAL_GetProductSecret(_OU_ char *product_secret)
{
    int len = strlen(_product_secret);
    memset(product_secret, 0x0, PRODUCT_SECRET_LEN);

#ifdef __DEMO__
    strncpy(product_secret, _product_secret, len);
#endif

    return len;
}

int HAL_GetDeviceName(_OU_ char *device_name)
{
    int len = strlen(_device_name);
    memset(device_name, 0x0, DEVICE_NAME_LEN);

#ifdef __DEMO__
    strncpy(device_name, _device_name, len);
#endif

    return strlen(device_name);
}

int HAL_GetDeviceSecret(_OU_ char *device_secret)
{
    int len = strlen(_device_secret);
    memset(device_secret, 0x0, DEVICE_SECRET_LEN);

#ifdef __DEMO__
    strncpy(device_secret, _device_secret, len);
#endif

    return len;
}

/*
 * This need to be same with app version as in uOTA module (ota_version.h)

    #ifndef SYSINFO_APP_VERSION
    #define SYSINFO_APP_VERSION "app-1.0.0-20180101.1000"
    #endif
 *
 */
int HAL_GetFirmwareVersion(_OU_ char *version)
{
    const char *ver = HAL_GetEAVerison();
    int len = strlen(ver);
    memset(version, 0x0, FIRMWARE_VERSION_MAXLEN);
#ifdef __DEMO__
    strncpy(version, ver, len);
    version[len] = '\0';
#endif
    return strlen(version);
}

const char* HAL_GetIEVerison(void)
{
    return IE_VER;
}

const char* HAL_GetEAVerison(void)
{
    return EA_VER;
}

void *HAL_SemaphoreCreate(void)
{
    sem_t *sem = (sem_t *)malloc(sizeof(sem_t));
    if (NULL == sem) {
        return NULL;
    }

    *sem =xSemaphoreCreateMutex();

    return sem;
}

void HAL_SemaphoreDestroy(_IN_ void *sem)
{
    vSemaphoreDelete(*(sem_t*)sem);
    free(sem);
}

void HAL_SemaphorePost(_IN_ void *sem)
{
    xSemaphoreGive(*(sem_t*)sem);
}

int HAL_SemaphoreWait(_IN_ void *sem, _IN_ uint32_t timeout_ms)
{
    return (xSemaphoreTake(*(sem_t*)sem, (portTickType)timeout_ms) == pdPASS) ? 0 : -1;
}

int HAL_ThreadCreate(
            _OU_ void **thread_handle,
            _IN_ void *(*work_routine)(void *),
            _IN_ void *arg,
            _IN_ hal_os_thread_param_t *hal_os_thread_param,
            _OU_ int *stack_used)
{
    int ret = -1;

    if (stack_used) {
        *stack_used = 0;
    }

    ret = pthread_create((pthread_t *)thread_handle, NULL, work_routine, arg);

    return ret;
}

void HAL_ThreadDetach(_IN_ void *thread_handle)
{
    pthread_detach((pthread_t)thread_handle);
}

void HAL_ThreadDelete(_IN_ void *thread_handle)
{
    if (NULL == thread_handle) {
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    	ESP_LOGE(OS_TAG, "%s: esp82666 not supported!", __FUNCTION__);
#else
        pthread_exit(0);
#endif
    } else {
        /*main thread delete child thread*/
        pthread_cancel((pthread_t)thread_handle);
        pthread_join((pthread_t)thread_handle, 0);
    }
}

esp_ota_handle_t update_handle = 0 ;
const esp_partition_t *update_partition = NULL;
uint32_t sum_download_bytes = 0;

void HAL_Firmware_Persistence_Start(void)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(OS_TAG, "Starting ESP-OTA...");
    update_handle = 0 ;
    update_partition = NULL;
    sum_download_bytes = 0;
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(OS_TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
                 configured->address, running->address);
        ESP_LOGW(OS_TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(OS_TAG, "Running partition type %d subtype %d (offset 0x%08x)",
             running->type, running->subtype, running->address);

    update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(OS_TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);
    assert(update_partition != NULL);
    //    assert(fp);
    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(OS_TAG, "esp_ota_begin failed, error=%d", err);
        return;
    }

    ESP_LOGI(OS_TAG, "esp_ota_begin succeeded");
    return;
}

int HAL_Firmware_Persistence_Write(_IN_ char *buffer, _IN_ uint32_t length)
{
    esp_err_t err = ESP_OK;
    err = esp_ota_write( update_handle, (const void *)buffer, length);
    if (err != ESP_OK) {
        ESP_LOGE(OS_TAG, "Error: esp_ota_write failed! err=0x%x", err);
        return -1;
    }

    sum_download_bytes += length;
    ESP_LOGI(OS_TAG, "%d w ok", sum_download_bytes);
    return length;
}

int HAL_Firmware_Persistence_Stop(void)
{
    ESP_LOGI(OS_TAG, "Stop ESP-OTA...");
    esp_err_t err = ESP_OK;
    if(update_handle != 0) {
        if (esp_ota_end(update_handle) != ESP_OK) {
            ESP_LOGE(OS_TAG, "esp_ota_end failed!");
            return -1;
        }
    }

    if(update_partition != NULL) {
        err = esp_ota_set_boot_partition(update_partition);
        if (err != ESP_OK) {
            ESP_LOGE(OS_TAG, "esp_ota_set_boot_partition failed! err=0x%x", err);
            return -1;
        }
    }

    ESP_LOGI(OS_TAG, "Prepare to restart system!");
    /* check file md5, and burning it to flash ... finally reboot system */
    esp_restart();
    return 0;
}

int HAL_Config_Write(const char *buffer, int length)
{
    FILE *fp;
    size_t written_len;
    char filepath[128] = {0};

    if (!buffer || length <= 0) {
        return -1;
    }

    snprintf(filepath, sizeof(filepath), "./%s", "alinkconf");
    fp = fopen(filepath, "w");
    if (!fp) {
        return -1;
    }

    written_len = fwrite(buffer, 1, length, fp);

    fclose(fp);

    return ((written_len != length) ? -1 : 0);
}

int HAL_Config_Read(char *buffer, int length)
{
    FILE *fp;
    size_t read_len;
    char filepath[128] = {0};

    if (!buffer || length <= 0) {
        return -1;
    }

    snprintf(filepath, sizeof(filepath), "./%s", "alinkconf");
    fp = fopen(filepath, "r");
    if (!fp) {
        return -1;
    }

    read_len = fread(buffer, 1, length, fp);
    fclose(fp);

    return ((read_len != length) ? -1 : 0);
}

void HAL_Reboot(void)
{
    esp_restart();
}

#define ROUTER_INFO_PATH        "/proc/net/route"
#define ROUTER_RECORD_SIZE      256


char *_get_default_routing_ifname(char *ifname, int ifname_size)
{
    return NULL;
#if 0
    FILE *fp = NULL;
    char line[ROUTER_RECORD_SIZE] = {0};
    char iface[IFNAMSIZ] = {0};
    char *result = NULL;
    unsigned int destination, gateway, flags, mask;
    unsigned int refCnt, use, metric, mtu, window, irtt;

    fp = fopen(ROUTER_INFO_PATH, "r");
    if (fp == NULL) {
        perror("fopen");
        return result;
    }

    char *buff = fgets(line, sizeof(line), fp);
    if (buff == NULL) {
        perror("fgets");
        goto out;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (11 !=
            sscanf(line, "%s %08x %08x %x %d %d %d %08x %d %d %d",
                   iface, &destination, &gateway, &flags, &refCnt, &use,
                   &metric, &mask, &mtu, &window, &irtt)) {
            perror("sscanf");
            continue;
        }

        /*default route */
        if ((destination == 0) && (mask == 0)) {
            strncpy(ifname, iface, ifname_size - 1);
            result = ifname;
            break;
        }
    }

out:
    if (fp) {
        fclose(fp);
    }

    return result;
#endif
}


uint32_t HAL_Wifi_Get_IP(char ip_str[NETWORK_ADDR_LEN], const char *ifname)
{
    if (ip_str == NULL) {
        return 0;
    }
    tcpip_adapter_ip_info_t info;
    tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &info);
    memcpy(ip_str, inet_ntoa(info.ip.addr), NETWORK_ADDR_LEN);

    return info.ip.addr;
}

static const char *NVS_KV = "kv data";

int HAL_Kv_Set(const char *key, const void *val, int len, int sync)
{
    nvs_handle handle;
    nvs_open( NVS_KV, NVS_READWRITE, &handle);
    nvs_set_blob( handle, key, val, len);
    nvs_close(handle);
    return 0;
}

int HAL_Kv_Get(const char *key, void *buffer, int *buffer_len)
{
    nvs_handle handle;
    nvs_open(NVS_KV, NVS_READWRITE, &handle);
    nvs_get_blob(handle, key, buffer, (size_t*)buffer_len);
    nvs_close(handle);
    return 0;
}

int HAL_Kv_Del(const char *key)
{
    nvs_handle handle;
    nvs_open( NVS_KV, NVS_READWRITE, &handle);
    nvs_erase_key( handle, key);
    nvs_close(handle);
    return 0;
}

static long long os_time_get(void)
{
    struct timeval tv;
    long long ms;
    gettimeofday(&tv, NULL);
    ms = tv.tv_sec * 1000LL + tv.tv_usec / 1000;
    return ms;
}

static long long delta_time = 0;

void HAL_UTC_Set(long long ms)
{
    delta_time = ms - os_time_get();
}

long long HAL_UTC_Get(void)
{
    return delta_time + os_time_get();
}

typedef struct {
    esp_timer_handle_t  timer;
} timer_handler;

void *HAL_Timer_Create(const char *name, void (*func)(void *), void *user_data)
{
    /* check parameter */
    if (func == NULL) {
        return NULL;
    }

    timer_handler *timer = (timer_handler *)malloc(sizeof(timer_handler));
    if(!timer) {
        return NULL;
    } else {
        memset(timer, 0x0, sizeof(timer_handler));
    }

    esp_timer_create_args_t timer_args = {
            .callback = func,
            .arg = user_data,
            .name = name
    };

    esp_timer_create(&timer_args, &(timer->timer));

    return (void *)timer;
}

int HAL_Timer_Start(void *input_timer, int ms)
{
    /* check parameter */
    if (input_timer == NULL) {
        return -1;
    }
    timer_handler *timer = (timer_handler *)input_timer;
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    ms = (ms == 1) ? 10 : ms;
#endif
    return esp_timer_start_periodic(timer->timer, ms * 1000);
}

int HAL_Timer_Stop(void *input_timer)
{
    /* check parameter */
    if (input_timer == NULL) {
        return -1;
    }

    timer_handler *timer = (timer_handler *)input_timer;

    return esp_timer_stop(timer->timer);
}

int HAL_Timer_Delete(void *input_timer)
{
    /* check parameter */
    if (input_timer == NULL) {
        return -1;
    }

    timer_handler *timer = (timer_handler *)input_timer;

    esp_err_t ret = esp_timer_delete(timer->timer);

    free(timer);
    timer = NULL;

    return ret;
}

int HAL_GetNetifInfo(char *nif_str)
{
#define MAC_ADDR_LEN    6

    static char* mac = NULL;

    if(mac == NULL) {
        mac = (char*)malloc(MAC_ADDR_LEN);
    }

    if(mac == NULL) {
        hal_err("malloc failed");
        return NULL;
    } else {
        memset(mac, 0x0, MAC_ADDR_LEN);
    }

    if( esp_wifi_get_mac(ESP_IF_WIFI_STA, (uint8_t*)mac) != ESP_OK) {
        return NULL;
    }

    return snprintf(nif_str, NIF_STRLEN_MAX, "WiFi|%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
