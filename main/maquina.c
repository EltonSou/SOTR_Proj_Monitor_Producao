#include "maquina.h"

uint8_t estadosMsg[ESTADOS_MAX][30] = {
            [Alimenta] = "Alimenta",
            [Esquenta] = "Esquenta",
            [Injeta]   = "Injeta",
            [Molda]    = "Molda",
            [Esfria]   = "Esfria",
            [Dispensa] = "Dispensa",
        };

void executaMaquina ( Maquina *maq) {
      
    if(DEBUG){
        ESP_LOGI("","\rMaquina: %s                 \n \
                     \rTempo de ciclo: %d          \n \
                     \rSensor Fim Ciclo: %d        \n \
                     \rSensor Maquina Parada: %d   \n \
                     \rSensor Maquina Desligada: %d\n \
                     \rEstado atual: %s            \n \
                     \rSaida injetora: %d          \n \
                     \rEspera: %d                  \n\
                     \rProximo: %s                 \n",
                     maq->id,
                     maq->tempoDeCiclo,
                     maq->sensorFimCiclo,
                     maq->sensorMaquinaParada,
                     maq->sensorMaquinaDesligada,
                     &estadosMsg[maq->actualState][0],
                     maq->fsm[maq->actualState].out,
                     maq->fsm[maq->actualState].wait,
                     &estadosMsg[maq->fsm[maq->actualState].next][0]);
    }

    vTaskDelay(pdMS_TO_TICKS(maq->fsm[maq->actualState].wait));

    maq->actualState = maq->fsm[maq->actualState].next;
}

