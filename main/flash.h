#ifndef __FLASH_H__
#define __FLASH_H__

#ifdef __cplusplus
    extern "C"{
#endif

    #include "main.h"

    void nvsFlashInit(void);
    BaseType_t nvsFlashRead(uint32_t *value, const char *value_name, const char *storage_name);
    BaseType_t nvsFlashSave(uint32_t value, const char *value_name, const char *storage_name);

#ifdef __cplusplus
    }
#endif

#endif