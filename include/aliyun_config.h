#ifndef ALIYUN_CONFIG_H_
#define ALIYUN_CONFIG_H_


#define MQTT_DIRECT

#define PRODUCT_KEY             "ymXuzyfmuQb"
#define DEVICE_NAME             "esp8266_test001"
#define DEVICE_SECRET           "32fZgRbygua80rdVNLb5femE7wHBh7G9"

// These are pre-defined topics
#define TOPIC_UPDATE            "/ymXuzyfmuQb/esp8266_test001/update"
#define TOPIC_ERROR             "/ymXuzyfmuQb/esp8266_test001/update/error"
#define TOPIC_GET               "/ymXuzyfmuQb/esp8266_test001/get"
#define TOPIC_DATA              "/ymXuzyfmuQb/esp8266_test001/data"
#define TOPIC_RELAY            "/ymXuzyfmuQb/esp8266_test001/relay"

#define WIFI_SSID       "BL_841R"
#define WIFI_PASSWORD   "1234567890"
#define MSG_LEN_MAX             (2048)

#define OTA_MODULE 1
#define OTA_BUF_LEN 1460

#if 1
#define EXAMPLE_TRACE(fmt, args...)  \
    do { \
        os_printf("%s|%03d :: ", __func__, __LINE__); \
        os_printf(fmt, ##args); \
        os_printf("%s", "\r\n"); \
    } while(0)
#else
#define EXAMPLE_TRACE(fmt, args...)  \
    os_printf(fmt, ##args)
#endif



#endif
