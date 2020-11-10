#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
    extern "C" {
#endif

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

/*
 * GPIO
 */
#include "driver/gpio.h"

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

/*
 * Maquina 
 */
#include "config.h"
#include "fsm.h"
#include "maquina.h"

/*
 * Protocolo de comunicação com o sistema Syntera da AutomaWay
 */
#include "syntera.h"

#ifdef __cplusplus
    }
#endif

#endif