#ifndef ESPNOW_COMMON_H
#define ESPNOW_COMMON_H

#include <stdio.h>
#include <string.h>
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Define pins and other constants
#define ESPNOW_TAG "ESP-NOW"

// Declare shared variables
extern bool paring;
extern uint8_t vest_mac[6];
extern uint8_t gun_mac[6];

// Data structure to hold the message to send
typedef struct
{
    char msg[32];
} espnow_data_t;

#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
void vest_espnow_recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len);
void blaster_espnow_recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len);
void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status);
void wifi_init();
void vest_espnow_init();
void blaster_espnow_init();
void get_own_mac_address(uint8_t *mac);
void vest_send_message(const char *message);
void blaster_send_message(const char *message);
void vest_setupESPnow();
void blaster_setupESPnow();
void send_pairing_message();
uint8_t getLife();
uint8_t getTeam();
void setLife(uint8_t newLife);
void setTeam(uint8_t newTeam);


#ifdef __cplusplus
}
#endif

#endif // ESPNOW_COMMON_H
