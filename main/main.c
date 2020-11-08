/*
  Autor: Vitor M. dos S. Alho
  Autor: Elton G. Souza
  Hardware: ESP32
  SDK-IDF: v3.3.4
  Disciplina: Sistema Operacional de Tempo Real (SOTR)
  Professor: Fernando Simplício
  Turma: 7SE- Sábado de manhã
*/

/**
 * Lib C
 */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/**
 * FreeRTOS
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

/**
 * WiFi
 */
#include "esp_wifi.h"

/**
 * WiFi Callback
 */
#include "esp_event_loop.h"

/**
 * Log
 */
#include "esp_system.h"
#include "esp_log.h"

/**
 * NVS
 */
#include "nvs.h"
#include "nvs_flash.h"

/**
 * LWIP
 */
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

/**
 * Lib MQTT
 */
#include "mqtt_client.h"

/**
 * Definições Gerais
 */
#define DEBUG 1
#define CONFIG_WIFI_SSID        "Alho_Home"
#define CONFIG_WIFI_PASSWORD    "P!nn@pl#"

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

}
