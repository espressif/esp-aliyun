/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
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

#include "wrappers_defs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "pthread.h"

#include "esp_log.h"
#include "esp_timer.h"


#ifdef CONFIG_IDF_TARGET_ESP8266
    static const char *TAG = "wrapper_os";
#endif
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
    return (void *)xSemaphoreCreateMutex();
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
    if (mutex) {
        vSemaphoreDelete((SemaphoreHandle_t)mutex);
    }
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
    if (mutex) {
        xSemaphoreTake((SemaphoreHandle_t)mutex, portMAX_DELAY);
    }
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
    if (mutex) {
        xSemaphoreGive((SemaphoreHandle_t)mutex);
    }
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
    return (void *)xSemaphoreCreateCounting(CONFIG_HAL_SEM_MAX_COUNT, CONFIG_HAL_SEM_INIT_COUNT);
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
    if (sem) {
        vSemaphoreDelete((SemaphoreHandle_t)sem);
    }
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
    if (sem) {
        xSemaphoreGive((SemaphoreHandle_t)sem);
    }
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
    if (pdPASS == xSemaphoreTake((SemaphoreHandle_t)sem, timeout_ms)) {
        return SUCCESS_RETURN;
    }

    return FAIL_RETURN;
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
    int ret = -1;

    if (stack_used) {
        *stack_used = 0;
    }

    ret = pthread_create((pthread_t *)thread_handle, NULL, work_routine, arg);

    return ret;

}

void HAL_ThreadDelete(void *thread_handle)
{
    if (NULL == thread_handle) {
#ifdef CONFIG_IDF_TARGET_ESP8266
        ESP_LOGE(TAG, "%s: esp82666 not supported!", __FUNCTION__);
#else
        pthread_exit(0);
#endif
    } else {
        /*main thread delete child thread*/
        pthread_cancel((pthread_t) thread_handle);
        pthread_join((pthread_t) thread_handle, 0);
    }

}

extern void HAL_ThreadDetach(void *thread_handle)
{
    pthread_detach((pthread_t)thread_handle);
}

void *HAL_Timer_Create(const char *name, void (*func)(void *), void *user_data)
{
    esp_timer_handle_t timer_handle = NULL;
    esp_timer_create_args_t timer_args = {
        .callback = func,
        .arg = user_data,
        .name = name
    };

    esp_timer_create(&timer_args, &timer_handle);

    return (void *)timer_handle;
}

int HAL_Timer_Delete(void *timer)
{
    if (ESP_OK == esp_timer_delete((esp_timer_handle_t)timer)) {
        return SUCCESS_RETURN;
    }

    return FAIL_RETURN;
}

int HAL_Timer_Start(void *timer, int ms)
{
#ifdef CONFIG_IDF_TARGET_ESP8266
    ms = (ms == 1) ? 10 : ms;
#endif

    if (ESP_OK == esp_timer_start_periodic((esp_timer_handle_t)timer, ms * 1000)) {
        return SUCCESS_RETURN;
    }

    return FAIL_RETURN;
}

int HAL_Timer_Stop(void *timer)
{
    if (ESP_OK == esp_timer_stop((esp_timer_handle_t)timer)) {
        return SUCCESS_RETURN;
    }

    return FAIL_RETURN;
}
