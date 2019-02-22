/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */
#include "sdkconfig.h"
#include <string.h>
#include "iot_import.h"
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
#include "esp_aes.h"
#else
#include "mbedtls/aes.h"
#endif

#define AES_BLOCK_SIZE 16

typedef struct {
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
	esp_aes_context     ctx;
#else
    mbedtls_aes_context ctx;
#endif
    uint8_t iv[16];
    uint8_t key[16];
} platform_aes_t;

p_HAL_Aes128_t HAL_Aes128_Init(
            _IN_ const uint8_t *key,
            _IN_ const uint8_t *iv,
            _IN_ AES_DIR_t dir)
{
    int ret = 0;
    platform_aes_t *p_aes128 = NULL;

    if (!key || !iv) return p_aes128;

    p_aes128 = (platform_aes_t *)calloc(1, sizeof(platform_aes_t));
    if (!p_aes128) return p_aes128;

#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    esp_aes_init(&p_aes128->ctx);
#else
    mbedtls_aes_init(&p_aes128->ctx);
#endif

    if (dir == HAL_AES_ENCRYPTION) {
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    	ret = esp_aes_setkey_enc(&p_aes128->ctx, key, 128);
#else
        ret = mbedtls_aes_setkey_enc(&p_aes128->ctx, key, 128);
#endif
    } else {
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    	ret = esp_aes_setkey_dec(&p_aes128->ctx, key, 128);
#else
        ret = mbedtls_aes_setkey_dec(&p_aes128->ctx, key, 128);
#endif
    }

    if (ret == 0) {
        memcpy(p_aes128->iv, iv, 16);
        memcpy(p_aes128->key, key, 16);
    } else {
        free(p_aes128);
        p_aes128 = NULL;
    }

    return (p_HAL_Aes128_t *)p_aes128;
}

int HAL_Aes128_Destroy(_IN_ p_HAL_Aes128_t aes)
{
    if (!aes) return -1;

#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    esp_aes_free(&((platform_aes_t *)aes)->ctx);
#else
    mbedtls_aes_free(&((platform_aes_t *)aes)->ctx);
#endif
    free(aes);

    return 0;
}

int HAL_Aes128_Cbc_Encrypt(
            _IN_ p_HAL_Aes128_t aes,
            _IN_ const void *src,
            _IN_ size_t blockNum,
            _OU_ void *dst)
{
    int i   = 0;
    int ret = ret;
    platform_aes_t *p_aes128 = (platform_aes_t *)aes;

    if (!aes || !src || !dst) return -1;

    for (i = 0; i < blockNum; ++i) {
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    	ret = esp_aes_crypt_cbc(&p_aes128->ctx, ESP_AES_ENCRYPT, AES_BLOCK_SIZE,
    	                                    p_aes128->iv, src, dst);
#else
        ret = mbedtls_aes_crypt_cbc(&p_aes128->ctx, MBEDTLS_AES_ENCRYPT, AES_BLOCK_SIZE,
                                    p_aes128->iv, src, dst);
#endif
        src += 16;
        dst += 16;
    }

    return ret;
}

int HAL_Aes128_Cbc_Decrypt(
            _IN_ p_HAL_Aes128_t aes,
            _IN_ const void *src,
            _IN_ size_t blockNum,
            _OU_ void *dst)
{
    int i   = 0;
    int ret = -1;
    platform_aes_t *p_aes128 = (platform_aes_t *)aes;

    if (!aes || !src || !dst) return ret;

    for (i = 0; i < blockNum; ++i) {
#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    	ret = esp_aes_crypt_cbc(&p_aes128->ctx, ESP_AES_DECRYPT, AES_BLOCK_SIZE,
    	                                    p_aes128->iv, src, dst);
#else
        ret = mbedtls_aes_crypt_cbc(&p_aes128->ctx, MBEDTLS_AES_DECRYPT, AES_BLOCK_SIZE,
                                    p_aes128->iv, src, dst);
#endif
        src += 16;
        dst += 16;
    }

    return ret;
}

int HAL_Aes128_Cfb_Encrypt(
            _IN_ p_HAL_Aes128_t aes,
            _IN_ const void *src,
            _IN_ size_t length,
            _OU_ void *dst)
{
    size_t offset = 0;
    int ret = -1;
    platform_aes_t *p_aes128 = (platform_aes_t *)aes;

    if (!aes || !src || !dst) return ret;

#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    ret = esp_aes_crypt_cfb128(&p_aes128->ctx, ESP_AES_ENCRYPT, length,
                                       &offset, p_aes128->iv, src, dst);
#else
    ret = mbedtls_aes_crypt_cfb128(&p_aes128->ctx, MBEDTLS_AES_ENCRYPT, length,
                                   &offset, p_aes128->iv, src, dst);
#endif
    return ret;
}

int HAL_Aes128_Cfb_Decrypt(
            _IN_ p_HAL_Aes128_t aes,
            _IN_ const void *src,
            _IN_ size_t length,
            _OU_ void *dst)
{
    size_t offset = 0;
    int ret = -1;
    platform_aes_t *p_aes128 = (platform_aes_t *)aes;

    if (!aes || !src || !dst) return ret;

#ifdef CONFIG_TARGET_PLATFORM_ESP8266
    ret = esp_aes_setkey_enc(&p_aes128->ctx, p_aes128->key, 128);
    ret = esp_aes_crypt_cfb128(&p_aes128->ctx, ESP_AES_DECRYPT, length,
                                       &offset, p_aes128->iv, src, dst);
#else
    ret = mbedtls_aes_setkey_enc(&p_aes128->ctx, p_aes128->key, 128);
    ret = mbedtls_aes_crypt_cfb128(&p_aes128->ctx, MBEDTLS_AES_DECRYPT, length,
                                   &offset, p_aes128->iv, src, dst);
#endif
    return ret;
}
