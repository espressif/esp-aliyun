#ifndef _WRAPPERS_EXTRA_H_
#define _WRAPPERS_EXTRA_H_

#include "infra_types.h"
#include "infra_defs.h"
#include <esp_event_loop.h>
#include <esp_wifi.h>

/**
 * Register a System Event handler for External Provisioning.
 *
 * If an external Provisioning mechanism is enabled, it may need the system events to decide 
 * what actions need to be taken. This API can be used for this. 
 *
 * @param[in] nw_prov_event_handler Network Provisioning System Event handler
 */
void HAL_Wifi_Register_Prov_Handler(system_event_cb_t nw_prov_event_handler);

/**
 * Unregister System Event handler for External Provisioning.
 *
 * If a system handler is registered for external provisioning using HAL_Wifi_Register_Prov_Handler(),
 * the same can be unregistered using this call. 
 */
void HAL_Wifi_Unregister_Prov_Handler(void);

/**
 * @brief Register system event callback handler
 *
 * In linkkit, the ESP Event loop is started internally and so, it registers
 * its own callback. However, in case an application needs to listen to the
 * same events, it can register a callback with the Core using this API.
 *
 * @param[in] cb System Event callback handler
 */
void HAL_Wifi_Register_System_Event(system_event_cb_t cb);

/**
 * @brief Check whether got IP or not in the expected time
 *
 * @param[in] timeout_ms The expected time
 * 
 * @return SUCCESS_RETURN on success
 * @return other on error
 */
int HAL_Wifi_Got_IP(size_t timeout_ms);

/**
 * @brief Configure to join an AP
 *
 * @param[in] ssid The ssid of AP
 * @param[in] ssid_len The ssid length of AP
 * @param[in] password The password of AP
 * @param[in] password_len The password length of AP
 * @param[in] timeout_ms The expected time
 * 
 * @return SUCCESS_RETURN on success
 * @return other on error
 */
int HAL_Wifi_Sta_Connect(const uint8_t *ssid, uint16_t ssid_len, const uint8_t *password, uint16_t password_len, size_t timeout_ms);

/**
 * @brief Check whether configure network or not
 * 
 * @return true on success
 * @return false on error
 */
bool HAL_Wifi_Is_Network_Configured(void);

/**
 * @brief Store the information of an AP
 *
 * @param[in] ssid The ssid of AP
 * @param[in] ssid_len The ssid length of AP
 * @param[in] password The password of AP
 * @param[in] password_len The password length of AP
 * 
 * @return SUCCESS_RETURN on success
 * @return other on error
 */
int HAL_Wifi_Save_Network(const uint8_t *ssid, size_t ssid_len, const uint8_t *password, size_t password_len);

/**
 * @brief Restore the information of an AP
 * 
 * @return SUCCESS_RETURN on success
 * @return other on error
 */
int HAL_Wifi_Del_Network(void);

/**
 * @brief Initialize the information of wifi module
 * 
 * @return SUCCESS_RETURN on success
 * @return other on error
 */
int HAL_Wifi_Init(void);

/**
 * @brief Initialize the information of key store
 * 
 * @return SUCCESS_RETURN on success
 * @return other on error
 */
int HAL_Kv_Init(void);

#endif

