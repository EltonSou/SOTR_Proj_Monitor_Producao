#ifndef MAQUINA_H
#define MAQUINA_H

    #ifdef __cplusplus
    extern "C" {
    #endif 

        #include "main.h"   

        #define ON 1
        #define OFF 0

        enum{
            Alimenta = 0,
            Esquenta,
            Injeta,
            Molda,
            Esfria,
            Dispensa,
            ESTADOS_MAX,
        }estados;

        struct Maq {
            uint8_t *id;
            uint32_t tempoDeCiclo;
            uint32_t sensorFimCiclo;
            uint32_t sensorMaquinaParada;
            uint32_t sensorMaquinaDesligada;
            tipoS *fsm;
            uint32_t actualState; 
        };

        typedef struct Maq Maquina;

        void executaMaquina ( Maquina *maq);

    #ifdef __cplusplus
    }
    #endif 
#endif