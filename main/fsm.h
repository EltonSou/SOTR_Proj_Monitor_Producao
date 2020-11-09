#ifndef FSM_H
#define FSM_H

#ifdef __cplusplus
    extern "C" {
#endif
        #include "main.h"
        
        struct State{
            uint32_t out;
            uint32_t wait;
            uint32_t next;                       
        };

        typedef const struct State tipoS;

#ifdef __cplusplus
    }
#endif

#endif

