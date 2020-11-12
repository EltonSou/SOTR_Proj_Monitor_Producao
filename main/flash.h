#ifndef __FLASH_H__
#define __FLASH_H__

#ifdef __cplusplus
    extern "C"{
#endif

    #include "main.h"

    void nvsFlashInit(void);
    BaseType_t nvsFlashRead(int32_t *valor);
    BaseType_t nvsFlashSave(int32_t valor);

#ifdef __cplusplus
    }
#endif

#endif