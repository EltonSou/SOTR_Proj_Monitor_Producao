/*
  Autor: Vitor M. dos S. Alho
  Autor: Elton G. Souza
  Hardware: ESP32
  SDK-IDF: v3.3.4
  Disciplina: Sistema Operacional de Tempo Real (SOTR)
  Professor: Fernando Simplício
  Turma: 7SE- Sábado de manhã
*/

#include "main.h"

/**
 * Definições Gerais
 */

tipoS Fsm[6] = {
                  [Alimenta] = {0,1000,Esquenta},
                  [Esquenta] = {0,1000,Injeta},
                  [Injeta] = {0,1000,Molda},
                  [Molda] = {0,1000,Esfria},
                  [Esfria] = {0,1000,Dispensa},
                  [Dispensa] = {0,1000,Alimenta},
               };

Maquina injetora;

uint8_t nomeMaquina[] = "Injetora";

/**
 * Variáveis Globais
 */
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
      .uri = "mqtt://mqtt.automaway.com.br:1883",
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

void iniciaMaquina ( Maquina *maq){
    maq->id = nomeMaquina;
    maq->tempoDeCiclo = 6000; // em mili segundos
    maq->sensorFimCiclo = OFF;
    maq->sensorMaquinaParada = OFF;
    maq->sensorMaquinaDesligada = OFF;
    maq->fsm = Fsm;
    maq->actualState = Alimenta;
}

void machineTask ( void *pvParameter ) {

    iniciaMaquina( &injetora );
   
    while( 1 ){

        executaMaquina( &injetora);

        vTaskDelay( pdMS_TO_TICKS( 1000 ) );

    }

    vTaskDelete( NULL );

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
                        
                        if(injetora.sensorMaquinaDesligada == ON){

                            injetora.sensorMaquinaDesligada = OFF;

                            //iniciaMaquina( &injetora );

                        }
                        else{

                            iniciaMaquina( &injetora );

                            injetora.sensorMaquinaDesligada = ON;                    

                        }
                        
                      break;
                      case BOTAO_PARADA_EMERGENCIA_MAQUINA:

                        if(injetora.sensorMaquinaDesligada == OFF){

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

    /*
    Inicialização da memória não volátil para armazenamento de dados (Non-volatile storage (NVS)).
    **Necessário para realização da calibração do PHY. 
    */
    esp_err_t ret = nvs_flash_init();
    nvs_flash_erase();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    gpioInit();
    
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

    if( (xTaskCreate( machineTask, "machineTask", 2048, NULL, 1, NULL) != pdTRUE) ) {
        if( DEBUG ) {
            ESP_LOGI(TAG,"error - Nao foi possivel alocar machineTask\r\n");
            return;
        }
    }

    
    

}
