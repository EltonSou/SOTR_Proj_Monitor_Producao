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
            uint32_t sensorMaquinaLigada;
            tipoS *fsm;
            uint32_t actualState; 
        };

        typedef struct Maq Maquina;

        enum {
            TEMPO_DE_CICLO = 0,
            SENSOR_FIM_CICLO,
            SENSOR_MAQ_PARADA,
            SENSOR_MAQ_DESLIGADA,
        }machineEventNumber;

        struct maqEvent{
            uint8_t eventId;
            uint32_t value;
        };

        typedef struct maqEvent machineEvent;

        void executaMaquina ( Maquina *maq, xQueueHandle *queueHandle);

    #ifdef __cplusplus
    }
    #endif 
#endif