/*

  Autor: Vitor M. dos S. Alho
  Autor: Elton G. Souza
  Hardware: ESP32
  SDK-IDF: v3.3.4
  Disciplina: Sistema Operacional de Tempo Real (SOTR)
  Professor: Fernando Simplício
  Turma: 7SE- Sábado de manhã

  Projeto: Monitoramento de máquina industrial para controle de produção.

*/

#include "main.h"

/**
 * Definições Gerais
 */

tipoS Fsm[ 6 ] = {
                  [ Alimenta ] = { 0, 1000, Esquenta },
                  [ Esquenta ] = { 0, 1000, Injeta },
                  [ Injeta ]   = { 0, 1000, Molda },
                  [ Molda ]    = { 0, 1000, Esfria },
                  [ Esfria ]   = { 0, 1000, Dispensa },
                  [ Dispensa ] = { 0, 1000, Alimenta },
               };

/**
 * Variáveis Globais
 */

Maquina injetora;

uint8_t nomeMaquina[] = "Injetora";

static const char *TAG = "main: ";

static EventGroupHandle_t wifi_event_group;

static EventGroupHandle_t mqtt_event_group;

const static int CONNECTED_BIT = BIT0;

esp_mqtt_client_handle_t client;

/**
 * Protótipos
 */
static esp_err_t wifi_event_handler( void *ctx, system_event_t *event );
static void wifi_init_sta( void );

void app_main( void );

/**
 * Função de callback do stack MQTT; 
 * Por meio deste callback é possível receber as notificações com os status
 * da conexão e dos tópicos assinados e publicados;
 */
static esp_err_t mqtt_event_handler( esp_mqtt_event_handle_t event )
{
    /**
     * Repare que a variável client, no qual armazena o Handle da conexão socket
     * do MQTT foi colocada como global; 
     */
    client = event->client;
    
    switch (event->event_id) 
    {

        /*
         * Este case foi adicionado a partir da versão 3.2 do SDK-IDF;
         * e é necessário ser adicionado devido as configurações de error do compilador;
         */
          case MQTT_EVENT_BEFORE_CONNECT: 

            if( DEBUG )
                ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
            
          break;

    	/**
    	 * Evento chamado quando o ESP32 se conecta ao broker MQTT, ou seja, 
    	 * caso a conexão socket TCP, SSL/TSL e autenticação com o broker MQTT
    	 * tenha ocorrido com sucesso, este evento será chamado informando que 
    	 * o ESP32 está conectado ao broker;
    	 */
        case MQTT_EVENT_CONNECTED:

        	if( DEBUG )
            	ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            /**
             * Se chegou aqui é porque o ESP32 está conectado ao Broker MQTT; 
             * A notificação é feita setando em nível 1 o bit CONNECTED_BIT da 
             * variável mqtt_event_group;
             */
			esp_mqtt_client_subscribe( client, "comando", 0 );
            xEventGroupSetBits( mqtt_event_group, CONNECTED_BIT );
            break;
        /**
         * Evento chamado quando o ESP32 for desconectado do broker MQTT;
         */
        case MQTT_EVENT_DISCONNECTED:

        	if( DEBUG )
            	ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");	
            /**
             * Se chegou aqui é porque o ESP32 foi desconectado do broker MQTT;
             */
			    xEventGroupClearBits(mqtt_event_group, CONNECTED_BIT);
          break;

        /**
         * O eventos seguintes são utilizados para notificar quando um tópico é
         * assinado pelo ESP32;
         */
        case MQTT_EVENT_SUBSCRIBED:
            break;
		
		/**
		 * Quando a assinatura de um tópico é cancelada pelo ESP32, 
		 * este evento será chamado;
		 */
        case MQTT_EVENT_UNSUBSCRIBED:
            break;
		
		/**
		 * Este evento será chamado quando um tópico for publicado pelo ESP32;
		 */
        case MQTT_EVENT_PUBLISHED:
        	
        	if( DEBUG )
            	ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
		
		/**
		 * Este evento será chamado quando uma mensagem chegar em algum tópico 
		 * assinado pelo ESP32;
		 */
        case MQTT_EVENT_DATA:

        	if( DEBUG )
        	{
 	            ESP_LOGI(TAG, "MQTT_EVENT_DATA"); 

 	            /**
 	             * Sabendo o nome do tópico que publicou a mensagem é possível
 	             * saber a quem data pertence;
 	             */
	            ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
	            ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);       		
        	}		
            break;
		
		/**
		 * Evento chamado quando ocorrer algum erro na troca de informação
		 * entre o ESP32 e o Broker MQTT;
		 */
        case MQTT_EVENT_ERROR:
        	if( DEBUG )
            	ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

/**
 * Configuração do stack MQTT; 
 */
static void mqtt_app_start( void )
{
    /**
     * Sem SSL/TLS
     */
    const esp_mqtt_client_config_t mqtt_cfg = {  		
      .uri = BROKER_MQTT,
      .event_handle = mqtt_event_handler,
  		.username = "teste",
  		.password = "teste",
    };

    /**
     * Carrega configuração do descritor e inicializa stack MQTT;
     */
    esp_mqtt_client_handle_t client = esp_mqtt_client_init( &mqtt_cfg );
    esp_mqtt_client_start(client);
}

/**
 * Função de callback do WiFi;
 */
static esp_err_t wifi_event_handler( void *ctx, system_event_t *event )
{
    switch( event->event_id ) 
    {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits( wifi_event_group, CONNECTED_BIT );
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits( wifi_event_group, CONNECTED_BIT );
            break;

        default:
            break;
    }
    return ESP_OK;
}

/**
 * Inicializa a rede WiFi em modo Station;
 */
static void wifi_init_sta( void )
{
    tcpip_adapter_init();

    ESP_ERROR_CHECK( esp_event_loop_init( wifi_event_handler, NULL ) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );
    ESP_ERROR_CHECK( esp_wifi_set_config( ESP_IF_WIFI_STA, &wifi_config ) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    if( DEBUG )
    {
     	ESP_LOGI(TAG, "start the WIFI SSID:[%s] password:[%s]", CONFIG_WIFI_SSID, "******");
    	ESP_LOGI(TAG, "Waiting for wifi");   	
    }

}

/*
 * Task para gerenciar a máquina operando
 */

void iniciaMaquina ( Maquina *maq ){
    maq->id = nomeMaquina;
    maq->tempoDeCiclo = 6000; // em mili segundos
    maq->sensorFimCiclo = OFF;
    maq->sensorMaquinaParada = OFF;
    maq->sensorMaquinaLigada = ON;
    maq->fsm = Fsm;
    maq->actualState = Alimenta;
}

xQueueHandle machine_event_queue = NULL;

void machineTask ( void *pvParameter ) {

    iniciaMaquina( &injetora );
   
    while( 1 ){

        executaMaquina( &injetora, &machine_event_queue );

        vTaskDelay( pdMS_TO_TICKS( 1000 ) );

    }

    vTaskDelete( NULL );

}

void machineMonitor ( void *pvParameter){

    char str[20];

    machineEvent eventoRecebido;

    int32_t pecasProduzidas = 0;

    volatile SynteraMsg mensagem_servidor;

    mensagem_servidor.numOperacao = NUMERO_OP;
    mensagem_servidor.numPecas = 0;
    mensagem_servidor.tempoDePeca;
    mensagem_servidor.statusMaqParada = OFF;
    mensagem_servidor.perdaAutomatica = 0;

    mensagem_servidor.statusCartao[0] = 'C';
    mensagem_servidor.statusCartao[1] = 'A';
    mensagem_servidor.statusCartao[2] = 'R';
    mensagem_servidor.statusCartao[3] = 'O';
    mensagem_servidor.statusCartao[4] = 'K';
    mensagem_servidor.statusCartao[5] = 0;

    mensagem_servidor.statusMaqLigada = OFF;

    mensagem_servidor.topicoMQTT = TOPICO_MQTT;

    //nvsFlashRead(&pecasProduzidas,"num_pecas", "storage");
    nvsFlashRead(&pecasProduzidas);

    printf("Valor de var: %d",pecasProduzidas);

    mensagem_servidor.numPecas = pecasProduzidas;

    while( 1 ) {

        xQueueReceive( machine_event_queue, &eventoRecebido, portMAX_DELAY);

        switch( eventoRecebido.eventId ){
            case TEMPO_DE_CICLO:
                mensagem_servidor.tempoDePeca = eventoRecebido.value;
            break;
            case SENSOR_MAQ_PARADA:
                mensagem_servidor.statusMaqParada = eventoRecebido.value;
            break;
            case SENSOR_MAQ_DESLIGADA:
                mensagem_servidor.statusMaqLigada = eventoRecebido.value;
            break;
            case SENSOR_FIM_CICLO:
                // Detecta sensor ON e incrementa peças produzidas
                if(eventoRecebido.value == ON){

                    pecasProduzidas++;  // Mais uma peça foi produzida

                    nvsFlashSave(pecasProduzidas);

                }
                mensagem_servidor.numPecas = pecasProduzidas;
            break;
        }

        // Funcao que envia a mensagem
        if(DEBUG || DEBUG_MACHINE_MONITOR){
            ESP_LOGI( TAG, "\n\n\rSyntera - AutomaWay Sistemas \
                            \n\rNumero da operacao: %d       \
                            \n\rNumero de pecas: %d          \
                            \n\rTempo de peca: %d            \
                            \n\rMaquina parada: %d           \
                            \n\rPerda automatica: %d         \
                            \n\rStatus Cartao: CAROK         \
                            \n\rMaquina ligada: %d          ",
                                mensagem_servidor.numOperacao,
                                mensagem_servidor.numPecas,
                                mensagem_servidor.tempoDePeca,
                                mensagem_servidor.statusMaqParada,
                                mensagem_servidor.perdaAutomatica,                                
                                mensagem_servidor.statusMaqLigada);
        }

        sprintf( str,"%d_%d_%d_%d_%d_CAROK_%d",
                    mensagem_servidor.numOperacao,
                    mensagem_servidor.numPecas,
                    mensagem_servidor.tempoDePeca,
                    mensagem_servidor.statusMaqParada,
                    mensagem_servidor.perdaAutomatica,                                
                    mensagem_servidor.statusMaqLigada);

        if( esp_mqtt_client_publish( client, mensagem_servidor.topicoMQTT, str, strlen( str ), 0, 0 ) == 0 )
        {

            if( DEBUG )
            ESP_LOGI( TAG, "MSG: %s .\r\n",str);
            
        }	

        vTaskDelay( pdMS_TO_TICKS( 200 ) );

    }

    vTaskDelete(NULL);

}

/*
 * Configuração do GPIO de entrada da máquina
 */

static xQueueHandle gpio_event_queue = NULL;

static void gpio_isr_handler( void* arg ){

    uint32_t gpio_num = ( uint32_t )arg;

    xQueueSendFromISR( gpio_event_queue, &gpio_num, NULL );    

}

void gpioInit ( void ){
  gpio_config_t gpioInputConfig;

    // Configuro os GPIO_0 e GPIO_4 como entrada, pull-up ligado e interrupção com borda de subida
    gpioInputConfig.pin_bit_mask = (1ULL << BOTAO_DESLIGA_MAQUINA) | (1ULL << BOTAO_PARADA_EMERGENCIA_MAQUINA);
    gpioInputConfig.mode = GPIO_MODE_INPUT;
    gpioInputConfig.pull_up_en = GPIO_PULLUP_ENABLE;
    gpioInputConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpioInputConfig.intr_type = GPIO_INTR_POSEDGE;

    gpio_config(&gpioInputConfig);

    gpio_event_queue = xQueueCreate(1,sizeof(uint32_t));

    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);

    gpio_isr_handler_add(BOTAO_DESLIGA_MAQUINA, gpio_isr_handler,(void*)BOTAO_DESLIGA_MAQUINA);
    
    gpio_isr_handler_add(BOTAO_PARADA_EMERGENCIA_MAQUINA, gpio_isr_handler,(void*)BOTAO_PARADA_EMERGENCIA_MAQUINA);
}

void gpioTask ( void *pvParameter ) {

    uint32_t gpio_num = 0;

    while( 1 ){

        if( xQueueReceive( gpio_event_queue, &gpio_num, portMAX_DELAY ) ) {

            if( gpio_get_level(gpio_num) == 1) {

                vTaskDelay(pdMS_TO_TICKS(50));

                if( gpio_get_level(gpio_num) == 1) {

                    if( DEBUG ){

                        ESP_LOGI( TAG, "Button: %d value: %d", gpio_num, gpio_get_level( gpio_num ) );

                    }

                    switch(gpio_num){
                      case BOTAO_DESLIGA_MAQUINA:
                        
                        if(injetora.sensorMaquinaLigada == ON){

                            iniciaMaquina( &injetora );

                            injetora.sensorMaquinaLigada = OFF;                           

                        }
                        else{

                            //iniciaMaquina( &injetora );

                            injetora.sensorMaquinaLigada = ON;                    

                        }
                        
                      break;
                      case BOTAO_PARADA_EMERGENCIA_MAQUINA:

                        if(injetora.sensorMaquinaLigada == ON){

                            if(injetora.sensorMaquinaParada == ON){

                                injetora.sensorMaquinaParada = OFF;

                            }
                            else{
                                injetora.sensorMaquinaParada = ON;
                            }

                        }
                        
                      break;
                    }
                }

            }

        }

        vTaskDelay(pdMS_TO_TICKS(1000));

    }

    vTaskDelete(NULL);

}

/**
 * Inicio da Aplicação;
 */
void app_main( void )
{
    
    gpioInit();

    nvsFlashInit();

    //nvsFlashSave(50);
//    nvsFlashRead(&var);

 //   printf("Valor de var: %d",var);
/*
    printf("\n");

    // Restart module
    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
*/
   // ESP_LOGI(TAG,"Valor de var: %d", var);

    /*
       Event Group do FreeRTOS. 
       Só podemos enviar ou ler alguma informação TCP quando a rede WiFi estiver configurada, ou seja, 
       somente após o aceite de conexão e a liberação do IP pelo roteador da rede.
    */
    wifi_event_group = xEventGroupCreate();

    /*
      O ESP32 está conectado ao broker MQTT? Para sabermos disso, precisamos dos event_group do FreeRTOS.
    */
    mqtt_event_group = xEventGroupCreate();

    /*
      Configura a rede WiFi.
    */
    wifi_init_sta();

    /**
     * Aguarda a conexão WiFi do ESP32 com o roteador;
     */
    xEventGroupWaitBits( wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY );

    /**
     * Inicializa e configura a rede;
     */
    mqtt_app_start();

    if( (xTaskCreate( gpioTask, "gpioTask", 2048, NULL, 1, NULL) != pdTRUE) ) {
        if( DEBUG ) {
            ESP_LOGI(TAG,"error - Nao foi possivel alocar gpioTask\r\n");
            return;
        }
    }

    machine_event_queue = xQueueCreate(10,sizeof(machineEvent));

    if( (xTaskCreate( machineTask, "machineTask", 2048, NULL, 1, NULL) != pdTRUE) ) {
        if( DEBUG ) {   
            ESP_LOGI(TAG,"error - Nao foi possivel alocar machineTask\r\n");
            return;
        }
    }

    if( (xTaskCreate( machineMonitor, "machineMonitor", 2048, NULL, 1, NULL) != pdTRUE) ) {
        if( DEBUG ) {
            ESP_LOGI(TAG,"error - Nao foi possivel alocar machineTask\r\n");
            return;
        }
    }    

}
