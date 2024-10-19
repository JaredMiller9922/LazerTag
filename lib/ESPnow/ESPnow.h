// espnow_communication.h

#ifndef ESPNOW_H
#define ESPNOW_H

#include <stdio.h>
#include <string.h>
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ESPNOW_TAG "ESP-NOW"

// Global variables
extern bool paring;
extern uint8_t own_mac[6];
extern uint8_t peer_mac[6];

// Data structure for ESP-NOW message
typedef struct {
    char msg[32];
} espnow_data_t;

// Function declarations
void espnow_recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len);
void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status);
void wifi_init(void);
void espnow_init(void);
void get_own_mac_address(uint8_t *mac);
void send_message(const char *message);
void setupESPnow(void);

#endif // ESPNOW_COMMUNICATION_H
