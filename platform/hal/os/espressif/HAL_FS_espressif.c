/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

void *HAL_Fopen(const char *path, const char *mode)
{
    return NULL;
#if 0
    return (void *)fopen(path, mode);
#endif
}

uint32_t HAL_Fread(void *buff, uint32_t size, uint32_t count, void *stream)
{
    return 0;
#if 0
    return fread(buff, (size_t)size, (size_t)count, (FILE *)stream);
#endif
}
uint32_t HAL_Fwrite(const void *ptr, uint32_t size, uint32_t count, void *stream)
{
    return 0;
#if 0
    return (uint32_t)fwrite(ptr, (size_t)size, (size_t)count, (FILE *)stream);
#endif
}

int HAL_Fseek(void *stream, long offset, int framewhere)
{
    return 0;
#if 0
    return fseek((FILE *)stream, offset, framewhere);
#endif
}

int HAL_Fclose(void *stream)
{
    return 0;
#if 0
    return fclose((FILE *)stream);
#endif
}

long HAL_Ftell(void *stream)
{
    return 0;
#if 0
    return ftell((FILE *)stream);
#endif
}
