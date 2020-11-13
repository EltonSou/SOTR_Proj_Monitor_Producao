#include "maquina.h"

uint8_t estadosMsg[ESTADOS_MAX][30] = {
            [Alimenta] = "Alimenta",
            [Esquenta] = "Esquenta",
            [Injeta]   = "Injeta",
            [Molda]    = "Molda",
            [Esfria]   = "Esfria",
            [Dispensa] = "Dispensa",
        };

void executaMaquina ( Maquina *maq, xQueueHandle *queueHandle) {

    machineEvent novoEvento;

    if(DEBUG || DEBUG_MACHINE){
        ESP_LOGI("","\rMaquina: %s                 \n \
                    \rTempo de ciclo: %d          \n \
                    \rSensor Fim Ciclo: %d        \n \
                    \rSensor Maquina Parada: %d   \n \
                    \rSensor Maquina Ligada: %d\n \
                    \rEstado atual: %s            \n \
                    \rSaida injetora: %d          \n \
                    \rEspera: %d                  \n\
                    \rProximo: %s                 \n",
                    maq->id,
                    maq->tempoDeCiclo,
                    maq->sensorFimCiclo,
                    maq->sensorMaquinaParada,
                    maq->sensorMaquinaLigada,
                    &estadosMsg[maq->actualState][0],
                    maq->fsm[maq->actualState].out,
                    maq->fsm[maq->actualState].wait,
                    &estadosMsg[maq->fsm[maq->actualState].next][0]);
    }    

    // Bloco que carrega informações na fila de monitoramento
    
    novoEvento.eventId = TEMPO_DE_CICLO;
    novoEvento.value   = maq->tempoDeCiclo;

    xQueueSend(*queueHandle, &novoEvento, portMAX_DELAY);

    novoEvento.eventId = SENSOR_FIM_CICLO;
    novoEvento.value   = maq->sensorFimCiclo;

    xQueueSend(*queueHandle, &novoEvento, portMAX_DELAY);

    novoEvento.eventId = SENSOR_MAQ_DESLIGADA;
    novoEvento.value   = maq->sensorMaquinaLigada;

    xQueueSend(*queueHandle, &novoEvento, portMAX_DELAY);

    novoEvento.eventId = SENSOR_MAQ_PARADA;
    novoEvento.value   = maq->sensorMaquinaParada;

    xQueueSend(*queueHandle, &novoEvento, portMAX_DELAY);

    ////////////////////////////////////////////////////////////////////

    if(maq->sensorMaquinaLigada == ON){ 

        vTaskDelay(pdMS_TO_TICKS(maq->fsm[maq->actualState].wait));

        if( maq->sensorMaquinaParada == OFF ){
            maq->actualState = maq->fsm[maq->actualState].next;
        }

        if( maq->actualState == Dispensa ) {
            maq->sensorFimCiclo = ON;
        }
        else{
            maq->sensorFimCiclo = OFF;
        }
        
    }
}

