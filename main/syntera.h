#ifndef _SYNTERA_H_
#define _SYNTERA_H_

#ifdef __cplusplus
    extern "C" {
#endif

    #include "main.h"

    struct synteraMsg{
        uint32_t numOperacao;
        uint32_t numPecas;
        uint32_t tempoDePeca;
        uint8_t statusMaqParada;
        uint8_t perdaAutomatica;
        uint8_t statusCartao[6];
        uint8_t statusMaqLigada;
        const char *topicoMQTT;
    };

    typedef struct synteraMsg SynteraMsg;

#ifdef __cplusplus
    }
#endif

#endif