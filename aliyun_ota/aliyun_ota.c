/*
 * ESPRSSIF MIT License
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

#include "iot_export_ota.h"

#include "esp_common.h"
#include "lwip/mem.h"
#include "aliyun_config.h"
#include "aliyun_ota.h"

struct upgrade_param {
    uint32 fw_bin_addr;
    uint16 fw_bin_sec;
    uint16 fw_bin_sec_num;
    uint16 fw_bin_sec_earse;
    uint8 extra;
    uint8 save[4];
    uint8 *buffer;
};

LOCAL struct upgrade_param *upgrade;

//extern SpiFlashChip *flashchip;

LOCAL bool OUT_OF_RANGE(uint16 erase_sec)
{
    uint8 spi_size_map = system_get_flash_size_map();
    uint16 sec_num = 0;
    uint16 start_sec = 0;
    

    if (spi_size_map == FLASH_SIZE_8M_MAP_512_512 || 
            spi_size_map ==FLASH_SIZE_16M_MAP_512_512 ||
            spi_size_map ==FLASH_SIZE_32M_MAP_512_512){
            start_sec = (system_upgrade_userbin_check() == USER_BIN2)? 1:129;
            sec_num = 123;
    } else if(spi_size_map == FLASH_SIZE_16M_MAP_1024_1024 || 
            spi_size_map == FLASH_SIZE_32M_MAP_1024_1024){
            start_sec = (system_upgrade_userbin_check() == USER_BIN2)? 1:257;
            sec_num = 251;
    } else {
            start_sec = (system_upgrade_userbin_check() == USER_BIN2)? 1:65;
            sec_num = 59;
    }
    if((erase_sec >= start_sec) &&(erase_sec <= (start_sec + sec_num)))
    {
        return false;
    } else {
        return true;
    }
    
}

/******************************************************************************
 * FunctionName : user_upgrade_internal
 * Description  : a
 * Parameters   :
 * Returns      :
*******************************************************************************/
LOCAL bool  
system_upgrade_internal(struct upgrade_param *upgrade, uint8 *data, u32 len)
{
    bool ret = false;
    uint16 secnm=0;
    if(data == NULL || len == 0)
    {
        return true;
    }

    /*got the sumlngth,erase all upgrade sector*/
    if(len > SPI_FLASH_SEC_SIZE ) {
        upgrade->fw_bin_sec_earse=upgrade->fw_bin_sec;

        secnm=((upgrade->fw_bin_addr + len)>>12) + (len&0xfff?1:0);
        while(upgrade->fw_bin_sec_earse != secnm) {
            taskENTER_CRITICAL();
            if( OUT_OF_RANGE( upgrade->fw_bin_sec_earse) )
            {
                os_printf("fw_bin_sec_earse:%d, Out of range\n",upgrade->fw_bin_sec_earse);
                break;
            
            }
            else
            {
                 spi_flash_erase_sector(upgrade->fw_bin_sec_earse);
                 upgrade->fw_bin_sec_earse++;
            }
            taskEXIT_CRITICAL();
            vTaskDelay(10 / portTICK_RATE_MS);
        }
        os_printf("flash erase over\n");
        return true;
    }
    
    upgrade->buffer = (uint8 *)zalloc(len + upgrade->extra);

    memcpy(upgrade->buffer, upgrade->save, upgrade->extra);
    memcpy(upgrade->buffer + upgrade->extra, data, len);

    len += upgrade->extra;
    upgrade->extra = len & 0x03;
    len -= upgrade->extra;

    if(upgrade->extra<=4)
        memcpy(upgrade->save, upgrade->buffer + len, upgrade->extra);
    else
        os_printf("ERR3:arr_overflow,%u,%d\n",__LINE__,upgrade->extra);

    do {
        if (upgrade->fw_bin_addr + len >= (upgrade->fw_bin_sec + upgrade->fw_bin_sec_num) * SPI_FLASH_SEC_SIZE) {
            printf("spi_flash_write exceed\n");
            break;
        }

        if (spi_flash_write(upgrade->fw_bin_addr, (uint32 *)upgrade->buffer, len) != SPI_FLASH_RESULT_OK) {
            printf("spi_flash_write failed\n");
            break;
        }
        
        ret = true;
        upgrade->fw_bin_addr += len;
    } while (0);

    free(upgrade->buffer);
    upgrade->buffer = NULL;
    return ret;
}

/******************************************************************************
 * FunctionName : system_get_fw_start_sec
 * Description  : a
 * Parameters   :
 * Returns      :
*******************************************************************************/
uint16 system_get_fw_start_sec()
{
    if(upgrade != NULL) {
        return upgrade->fw_bin_sec;
    } else {
        return 0;
    }
}

/******************************************************************************
 * FunctionName : user_upgrade
 * Description  : a
 * Parameters   :
 * Returns      :
*******************************************************************************/
bool system_upgrade(uint8 *data, uint32 len)
{
    bool ret;
    ret = system_upgrade_internal(upgrade, data, len);
    return ret;
}

LOCAL void upgrade_recycle(void)
{
    system_upgrade_deinit();
    system_upgrade_reboot(); // if need
}


/******************************************************************************
 * FunctionName : system_upgrade_init
 * Description  : a
 * Parameters   :
 * Returns      :
*******************************************************************************/
void  
system_upgrade_init(void)
{
    uint32 user_bin2_start,user_bin1_start;
    uint8 spi_size_map = system_get_flash_size_map();
    
    if (upgrade == NULL) {
        upgrade = (struct upgrade_param *)zalloc(sizeof(struct upgrade_param));
    }
    
    user_bin1_start = 1; 

    if (spi_size_map == FLASH_SIZE_8M_MAP_512_512 || 
            spi_size_map ==FLASH_SIZE_16M_MAP_512_512 ||
            spi_size_map ==FLASH_SIZE_32M_MAP_512_512){
            user_bin2_start = 129;
            upgrade->fw_bin_sec_num = 123;
    } else if(spi_size_map == FLASH_SIZE_16M_MAP_1024_1024 || 
            spi_size_map == FLASH_SIZE_32M_MAP_1024_1024){
            user_bin2_start = 257;
            upgrade->fw_bin_sec_num = 251;
    } else {
            user_bin2_start = 65;
            upgrade->fw_bin_sec_num = 59;
    }
   
    upgrade->fw_bin_sec = (system_upgrade_userbin_check() == USER_BIN1) ? user_bin2_start : user_bin1_start;

    upgrade->fw_bin_addr = upgrade->fw_bin_sec * SPI_FLASH_SEC_SIZE;
    
    upgrade->fw_bin_sec_earse = upgrade->fw_bin_sec;
}

/******************************************************************************
 * FunctionName : system_upgrade_deinit
 * Description  : a
 * Parameters   :
 * Returns      :
*******************************************************************************/
void  
system_upgrade_deinit(void)
{
    if (upgrade != NULL) {
        free(upgrade);
        upgrade = NULL;
    }else {
        return;
    }
}

void print_debug(const char* data, const int len, const char* note){
#define COUNT_BYTE_AND_NEW_LINE 0
#define ALL_BINARY_SHOW 0
    printf("\n********** %s [len:%d] start addr:%p **********\n", note, len, data);
    int i = 0;
    for (i = 0; i < len; ++i){
#if !(ALL_BINARY_SHOW)
    if(data[i] < 33 || data[i] > 126)
     {
        if(i > 0 && (data[i-1] >= 33 && data[i-1] <= 126) )
                printf(" ");
        printf("%02x ",data[i]);
     }else{
        printf("%c", data[i]);
     }
#else
        printf("%02x ",data[i]);
#endif

#if COUNT_BYTE_AND_NEW_LINE
   if ((i + 1) % 32 == 0){
        printf("    | %d Bytes\n",i + 1);
    }
#endif
}

printf("\n---------- %s End ----------\n", note);
}



void ota_main(void *pvParameter){
    printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    printf("param:%p\n", pvParameter);
    int ota_over = 0;
    char buf_ota[OTA_BUF_LEN] = { 0 };
    void* pclient = pvParameter, *h_ota = NULL;
    h_ota = IOT_OTA_Init(PRODUCT_KEY, DEVICE_NAME, pclient);
    if (NULL == h_ota) {
        printf("h_ota is NULL,task deleting...\n");
        vTaskDelete(NULL);
    }
    printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    if (0 != IOT_OTA_ReportVersion(h_ota, "iotx_ver_1.0.0")) {
        printf("IOT_OTA_ReportVersion error,task deleting...\n");
        vTaskDelete(NULL);
    }
    printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    system_upgrade_init();
    printf("file:%s function:%s line:%d heap size:%d\n",__FILE__,__FUNCTION__,__LINE__, system_get_free_heap_size());
    do {
        uint32_t firmware_valid;

        EXAMPLE_TRACE("wait ota upgrade command....");

        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        if (IOT_OTA_IsFetching(h_ota)) {
            uint32_t last_percent = 0, percent = 0;
            char version[128], md5sum[33];
            uint32_t len, size_downloaded, size_file;
            do {

                len = IOT_OTA_FetchYield(h_ota, buf_ota, OTA_BUF_LEN, 1);
                if (len > 0) {
                    print_debug(buf_ota, len, "receive data from console");
                    if (system_upgrade(buf_ota, len) == false){
                        printf("system_upgrade error happended!\n");
                        vTaskDelete(NULL);
                    }
                }

                /* get OTA information */
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FETCHED_SIZE, &size_downloaded, 4);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FILE_SIZE, &size_file, 4);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_MD5SUM, md5sum, 33);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_VERSION, version, 128);

                last_percent = percent;
                percent = (size_downloaded * 100) / size_file;
                if (percent - last_percent > 0) {
                    IOT_OTA_ReportProgress(h_ota, percent, NULL);
                    IOT_OTA_ReportProgress(h_ota, percent, "hello");
                }
                IOT_MQTT_Yield(pclient, 100);
            }while(!IOT_OTA_IsFetchFinish(h_ota));

//            IOT_OTA_Ioctl(h_ota, IOT_OTAG_CHECK_FIRMWARE, &firmware_valid, 4);
//            if (0 == firmware_valid) {
//                EXAMPLE_TRACE("The firmware is invalid");
//            } else {
//                EXAMPLE_TRACE("The firmware is valid");
//
//            }

            ota_over = 1;
        }
        HAL_SleepMs(2000);
    } while (!ota_over);

    upgrade_recycle();

    HAL_SleepMs(1000);
    vTaskDelete(NULL);
}
