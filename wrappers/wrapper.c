/**
 * NOTE:
 *
 * HAL_TCP_xxx API reference implementation: wrappers/os/ubuntu/HAL_TCP_linux.c
 *
 */
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "wrappers_defs.h"
#include "iot_import_awss.h"
#include "os.h"
#include "stdarg.h"

int HAL_Aes128_Cbc_Decrypt(
            p_HAL_Aes128_t aes,
            const void *src,
            size_t blockNum,
            void *dst)
{
	return (int)1;
}


int HAL_Aes128_Cbc_Encrypt(
            p_HAL_Aes128_t aes,
            const void *src,
            size_t blockNum,
            void *dst)
{
	return (int)1;
}


DLL_HAL_API int HAL_Aes128_Cfb_Decrypt(
            _IN_ p_HAL_Aes128_t aes,
            _IN_ const void *src,
            _IN_ size_t length,
            _OU_ void *dst)
{
	return (int)1;
}


DLL_HAL_API int HAL_Aes128_Cfb_Encrypt(
            _IN_ p_HAL_Aes128_t aes,
            _IN_ const void *src,
            _IN_ size_t length,
            _OU_ void *dst)
{
	return (int)1;
}


int HAL_Aes128_Destroy(p_HAL_Aes128_t aes)
{
	return (int)1;
}


p_HAL_Aes128_t HAL_Aes128_Init(
            const uint8_t *key,
            const uint8_t *iv,
            AES_DIR_t dir)
{
	return (p_HAL_Aes128_t)1;
}


void HAL_Awss_Close_Monitor(void)
{
	return;
}


int HAL_Awss_Connect_Ap(
            _IN_ uint32_t connection_timeout_ms,
            _IN_ char ssid[HAL_MAX_SSID_LEN],
            _IN_ char passwd[HAL_MAX_PASSWD_LEN],
            _IN_OPT_ enum AWSS_AUTH_TYPE auth,
            _IN_OPT_ enum AWSS_ENC_TYPE encry,
            _IN_OPT_ uint8_t bssid[ETH_ALEN],
            _IN_OPT_ uint8_t channel)
{
	return (int)1;
}


int HAL_Awss_Get_Channelscan_Interval_Ms(void)
{
	return (int)1;
}




int HAL_Awss_Get_Timeout_Interval_Ms(void)
{
	return (int)1;
}


int HAL_Awss_Open_Ap(const char *ssid, const char *passwd, int beacon_interval, int hide)
{
	return (int)1;
}


void HAL_Awss_Open_Monitor(_IN_ awss_recv_80211_frame_cb_t cb)
{
	return;
}


void HAL_Awss_Switch_Channel(char primary_channel, char secondary_channel, uint8_t bssid[ETH_ALEN])
{
	return;
}


/**
 * @brief Set malloc/free function.
 *
 * @param [in] hooks: @n Specify malloc/free function you want to use
 *
 * @retval DTLS_SUCCESS : Success.
   @retval        other : Fail.
 * @see None.
 * @note None.
 */
DLL_HAL_API int HAL_DTLSHooks_set(dtls_hooks_t *hooks)
{
	return (int)1;
}


 /**
 * @brief Establish a DSSL connection.
 *
 * @param [in] p_options: @n Specify paramter of DTLS
   @verbatim
           p_host : @n Specify the hostname(IP) of the DSSL server
             port : @n Specify the DSSL port of DSSL server
    p_ca_cert_pem : @n Specify the root certificate which is PEM format.
   @endverbatim
 * @return DSSL handle.
 * @see None.
 * @note None.
 */
DLL_HAL_API DTLSContext *HAL_DTLSSession_create(coap_dtls_options_t  *p_options)
{
	return (DTLSContext*)1;
}


 /**
 * @brief Destroy the specific DSSL connection.
 *
 * @param[in] context: @n Handle of the specific connection.
 *
 * @return The result of free dtls session
 * @retval DTLS_SUCCESS : Read success.
 * @retval DTLS_INVALID_PARAM : Invalid parameter.
 * @retval DTLS_INVALID_CA_CERTIFICATE : Invalid CA Certificate.
 * @retval DTLS_HANDSHAKE_IN_PROGRESS : Handshake in progress.
 * @retval DTLS_HANDSHAKE_FAILED : Handshake failed.
 * @retval DTLS_FATAL_ALERT_MESSAGE : Recv peer fatal alert message.
 * @retval DTLS_PEER_CLOSE_NOTIFY : The DTLS session was closed by peer.
 * @retval DTLS_SESSION_CREATE_FAILED : Create session fail.
 * @retval DTLS_READ_DATA_FAILED : Read data fail.
 */DLL_HAL_API unsigned int HAL_DTLSSession_free(DTLSContext *context)
{
	return (unsigned)1;
}


 /**
 * @brief Read data from the specific DSSL connection with timeout parameter.
 *        The API will return immediately if len be received from the specific DSSL connection.
 *
 * @param [in] context @n A descriptor identifying a DSSL connection.
 * @param [in] p_data @n A pointer to a buffer to receive incoming data.
 * @param [in] p_datalen @n The length, in bytes, of the data pointed to by the 'p_data' parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block 'timeout_ms' millisecond maximumly.
 * @return The result of read data from DSSL connection
 * @retval DTLS_SUCCESS : Read success.
 * @retval DTLS_FATAL_ALERT_MESSAGE : Recv peer fatal alert message.
 * @retval DTLS_PEER_CLOSE_NOTIFY : The DTLS session was closed by peer.
 * @retval DTLS_READ_DATA_FAILED : Read data fail.
 * @see None.
 */
DLL_HAL_API unsigned int HAL_DTLSSession_read(DTLSContext *context,
        unsigned char *p_data,
        unsigned int *p_datalen,
        unsigned int timeout_ms)
{
	return (unsigned)1;
}


 /**
 * @brief Write data into the specific DSSL connection.
 *
 * @param [in] context @n A descriptor identifying a connection.
 * @param [in] p_data @n A pointer to a buffer containing the data to be transmitted.
 * @param [in] p_datalen @n The length, in bytes, of the data pointed to by the 'p_data' parameter.
 * @retval DTLS_SUCCESS : Success.
   @retval        other : Fail.
 * @see None.
 */
DLL_HAL_API unsigned int HAL_DTLSSession_write(DTLSContext *context,
        const unsigned char *p_data,
        unsigned int *p_datalen)
{
	return (unsigned)1;
}


extern int HAL_Fclose(void *stream)
{
	return (int)1;
}


    void HAL_Firmware_Persistence_Start(void)
{
	return;
}


    int HAL_Firmware_Persistence_Stop(void)
{
	return (int)1;
}


    int HAL_Firmware_Persistence_Write(char *buffer, uint32_t length)
{
	return (int)1;
}


extern void *HAL_Fopen(const char *path, const char *mode)
{
	return (void*)1;
}


extern uint32_t HAL_Fread(void *buff, uint32_t size, uint32_t count, void *stream)
{
	return (uint32_t)1;
}


/**
 * @brief Deallocate memory block
 *
 * @param[in] ptr @n Pointer to a memory block previously allocated with platform_malloc.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_Free(void *ptr)
{
	return;
}


extern int HAL_Fseek(void *stream, long offset, int framewhere)
{
	return (int)1;
}


extern long HAL_Ftell(void *stream)
{
	return (long)1;
}


extern uint32_t HAL_Fwrite(const void *ptr, uint32_t size, uint32_t count, void *stream)
{
	return (uint32_t)1;
}


/**
 * @brief Get device name from user's system persistent storage
 *
 * @param [ou] device_name: array to store device name, max length is IOTX_DEVICE_NAME_LEN
 * @return the actual length of device name
 */
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN + 1])
{
	return (int)1;
}


/**
 * @brief Get device secret from user's system persistent storage
 *
 * @param [ou] device_secret: array to store device secret, max length is IOTX_DEVICE_SECRET_LEN
 * @return the actual length of device secret
 */
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN + 1])
{
	return (int)1;
}


/**
 * @brief Get firmware version
 *
 * @param [ou] version: array to store firmware version, max length is IOTX_FIRMWARE_VER_LEN
 * @return the actual length of firmware version
 */
int HAL_GetFirmwareVersion(char *version)
{
	return (int)1;
}


/**
 * @brief Get product key from user's system persistent storage
 *
 * @param [ou] product_key: array to store product key, max length is IOTX_PRODUCT_KEY_LEN
 * @return  the actual length of product key
 */
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN + 1])
{
	return (int)1;
}


int HAL_GetProductSecret(char product_secret[IOTX_PRODUCT_SECRET_LEN + 1])
{
	return (int)1;
}


int HAL_Kv_Del(const char *key)
{
	return (int)1;
}


int HAL_Kv_Get(const char *key, void *val, int *buffer_len)
{
	return (int)1;
}


int HAL_Kv_Set(const char *key, const void *val, int len, int sync)
{
	return (int)1;
}


/**
 * @brief Allocates a block of size bytes of memory, returning a pointer to the beginning of the block.
 *
 * @param [in] size @n specify block size in bytes.
 * @return A pointer to the beginning of the block.
 * @see None.
 * @note Block value is indeterminate.
 */
void *HAL_Malloc(uint32_t size)
{
	return (void*)1;
}


/**
 * @brief Create a mutex.
 *
 * @retval NULL : Initialize mutex failed.
 * @retval NOT_NULL : The mutex handle.
 * @see None.
 * @note None.
 */
void *HAL_MutexCreate(void)
{
	return (void*)1;
}


/**
 * @brief Destroy the specified mutex object, it will release related resource.
 *
 * @param [in] mutex @n The specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexDestroy(void *mutex)
{
	return;
}


/**
 * @brief Waits until the specified mutex is in the signaled state.
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexLock(void *mutex)
{
	return;
}


/**
 * @brief Releases ownership of the specified mutex object..
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexUnlock(void *mutex)
{
	return;
}


/**
 * @brief Writes formatted data to stream.
 *
 * @param [in] fmt: @n String that contains the text to be written, it can optionally contain embedded format specifiers
     that specifies how subsequent arguments are converted for output.
 * @param [in] ...: @n the variable argument list, for formatted and inserted in the resulting string replacing their respective specifiers.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_Printf(const char *fmt, ...)
{
	return;
}


uint32_t HAL_Random(uint32_t region)
{
	return (uint32_t)1;
}


extern void *HAL_Realloc(void *ptr, uint32_t size)
{
	return (void*)1;
}


void HAL_Reboot()
{
	return;
}


/**
 * @brief   create a semaphore
 *
 * @return semaphore handle.
 * @see None.
 * @note The recommended value of maximum count of the semaphore is 255.
 */
void *HAL_SemaphoreCreate(void)
{
	return (void*)1;
}


/**
 * @brief   destory a semaphore
 *
 * @param[in] sem @n the specified sem.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SemaphoreDestroy(void *sem)
{
	return;
}


/**
 * @brief   signal thread wait on a semaphore
 *
 * @param[in] sem @n the specified semaphore.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SemaphorePost(void *sem)
{
	return;
}


/**
 * @brief   wait on a semaphore
 *
 * @param[in] sem @n the specified semaphore.
 * @param[in] timeout_ms @n timeout interval in millisecond.
     If timeout_ms is PLATFORM_WAIT_INFINITE, the function will return only when the semaphore is signaled.
 * @return
   @verbatim
   =  0: The state of the specified object is signaled.
   =  -1: The time-out interval elapsed, and the object's state is nonsignaled.
   @endverbatim
 * @see None.
 * @note None.
 */
int HAL_SemaphoreWait(void *sem, uint32_t timeout_ms)
{
	return (int)1;
}


int HAL_SetDeviceName(char *device_name)
{
	return (int)1;
}


int HAL_SetDeviceSecret(char *device_secret)
{
	return (int)1;
}


int HAL_SetProductKey(char *product_key)
{
	return (int)1;
}


int HAL_SetProductSecret(char *product_secret)
{
	return (int)1;
}


/**
 * @brief Sleep thread itself.
 *
 * @param [in] ms @n the time interval for which execution is to be suspended, in milliseconds.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SleepMs(uint32_t ms)
{
	return;
}


/**
 * @brief Writes formatted data to string.
 *
 * @param [out] str: @n String that holds written text.
 * @param [in] len: @n Maximum length of character will be written
 * @param [in] fmt: @n Format that contains the text to be written, it can optionally contain embedded format specifiers
     that specifies how subsequent arguments are converted for output.
 * @param [in] ...: @n the variable argument list, for formatted and inserted in the resulting string replacing their respective specifiers.
 * @return bytes of character successfully written into string.
 * @see None.
 * @note None.
 */
int HAL_Snprintf(char *str, const int len, const char *fmt, ...)
{
	return (int)1;
}


void HAL_Srandom(uint32_t seed)
{
	return;
}


    int32_t HAL_SSL_Destroy(uintptr_t handle)
{
	return (int32_t)1;
}


    uintptr_t HAL_SSL_Establish(const char *host, uint16_t port, const char *ca_crt, uint32_t ca_crt_len)
{
	return (uintptr_t)1;
}


    int HAL_SSLHooks_set(ssl_hooks_t *hooks)
{
	return (int)1;
}


    int HAL_SSL_Read(uintptr_t handle, char *buf, int len, int timeout_ms)
{
	return (int)1;
}


    int HAL_SSL_Write(uintptr_t handle, const char *buf, int len, int timeout_ms)
{
	return (int)1;
}


int HAL_Sys_Net_Is_Ready()
{
	return (int)1;
}


/**
 * @brief  create a thread
 *
 * @param[out] thread_handle @n The new thread handle, memory allocated before thread created and return it, free it after thread joined or exit.
 * @param[in] start_routine @n A pointer to the application-defined function to be executed by the thread.
        This pointer represents the starting address of the thread.
 * @param[in] arg @n A pointer to a variable to be passed to the start_routine.
 * @param[in] hal_os_thread_param @n A pointer to stack params.
 * @param[out] stack_used @n if platform used stack buffer, set stack_used to 1, otherwise set it to 0.
 * @return
   @verbatim
     = 0: on success.
     = -1: error occur.
   @endverbatim
 * @see None.
 * @note None.
 */
int HAL_ThreadCreate(
            void **thread_handle,
            void *(*work_routine)(void *),
            void *arg,
            hal_os_thread_param_t *hal_os_thread_param,
            int *stack_used)
{
	return (int)1;
}


void HAL_ThreadDelete(void *thread_handle)
{
	return;
}


extern void HAL_ThreadDetach(void *thread_handle)
{
	return;
}


void *HAL_Timer_Create(const char *name, void (*func)(void *), void *user_data)
{
	return (void*)1;
}


int HAL_Timer_Delete(void *timer)
{
	return (int)1;
}


int HAL_Timer_Start(void *timer, int ms)
{
	return (int)1;
}


int HAL_Timer_Stop(void *timer)
{
	return (int)1;
}


int HAL_UDP_close_without_connect(intptr_t sockfd)
{
	return (int)1;
}


intptr_t HAL_UDP_create(char *host, unsigned short port)
{
	return (intptr_t)1;
}


intptr_t HAL_UDP_create_without_connect(const char *host, unsigned short port)
{
	return (intptr_t)1;
}


int HAL_UDP_joinmulticast(intptr_t sockfd,
                          char *p_group)
{
	return (int)1;
}


int HAL_UDP_readTimeout(intptr_t p_socket,
                        unsigned char *p_data,
                        unsigned int datalen,
                        unsigned int timeout)
{
	return (int)1;
}


int HAL_UDP_recvfrom(intptr_t sockfd,
                     NetworkAddr *p_remote,
                     unsigned char *p_data,
                     unsigned int datalen,
                     unsigned int timeout_ms)
{
	return (int)1;
}


int HAL_UDP_sendto(intptr_t sockfd,
                   const NetworkAddr *p_remote,
                   const unsigned char *p_data,
                   unsigned int datalen,
                   unsigned int timeout_ms)
{
	return (int)1;
}


int HAL_UDP_write(intptr_t p_socket,
                  const unsigned char *p_data,
                  unsigned int datalen)
{
	return (int)1;
}


/**
 * @brief Retrieves the number of milliseconds that have elapsed since the system was boot.
 *
 * @return the number of milliseconds.
 * @see None.
 * @note None.
 */
uint64_t HAL_UptimeMs(void)
{
	return (uint64_t)1;
}


int HAL_Vsnprintf(char *str, const int len, const char *format, va_list ap)
{
	return (int)1;
}


int HAL_Wifi_Enable_Mgmt_Frame_Filter(
            _IN_ uint32_t filter_mask,
            _IN_OPT_ uint8_t vendor_oui[3],
            _IN_ awss_wifi_mgmt_frame_cb_t callback)
{
	return (int)1;
}


int HAL_Wifi_Get_Ap_Info(char ssid[HAL_MAX_SSID_LEN],char passwd[HAL_MAX_PASSWD_LEN],uint8_t bssid[ETH_ALEN])
{
	return (int)1;
}


uint32_t HAL_Wifi_Get_IP(char ip_str[NETWORK_ADDR_LEN], const char *ifname)
{
	return (uint32_t)1;
}


char *HAL_Wifi_Get_Mac(char mac_str[HAL_MAC_LEN])
{
	return (char*)1;
}


DLL_HAL_API int HAL_Wifi_Scan(awss_wifi_scan_result_cb_t cb)
{
	return (int)1;
}


int HAL_Wifi_Send_80211_Raw_Frame(_IN_ enum HAL_Awss_Frame_Type type,
                                  _IN_ uint8_t *buffer, _IN_ int len)
{
	return (int)1;
}


