
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
#include "esp_log.h"

#define BUTTON_PIN 48 // GPIO for the button
#define LED_PIN 38    // GPIO for the LED

#define ESPNOW_TAG "ESP-NOW"

// Paring in this state the gun will send out its ir encoded mac, and also await to recieve data through ir that contains its own mac along with a vest mac then init espnow
bool paring = true;
bool receivedGunAddress = false;
// Define your own MAC address array
uint8_t own_mac[6] = {0};

// Define the peer MAC address (this will be the other mac address out of the gun vest pare)
// uint8_t gun_mac[6] = {0x24, 0x58, 0x7c, 0xd6, 0xc1, 0x44}; // black
uint8_t gun_mac[6]; // gun red team
// const uint8_t peer_mac[6] = {0x24, 0x58, 0x7c, 0xd6, 0xbd, 0x14}; // white

// Data structure to hold the message to send
typedef struct
{
    char msg[32];
} espnow_data_t;

// Callback function to handle the received data
void espnow_recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    ESP_LOGI(ESPNOW_TAG, "Received message from: " MACSTR, MAC2STR(info->src_addr));
    ESP_LOGI(ESPNOW_TAG, "Received data: %.*s", len, data);    
}

// Callback function to handle the status of sending data
void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    ESP_LOGI(ESPNOW_TAG, "Send status: %s to " MACSTR, status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail", MAC2STR(mac_addr));
}

// Function to initialize Wi-Fi
void wifi_init()
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); // Set Wi-Fi to station mode
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(ESPNOW_TAG, "Wi-Fi initialized successfully");
}

void espnow_init()
{
    ESP_ERROR_CHECK(esp_now_init());
    ESP_LOGI(ESPNOW_TAG, "ESP-NOW initialized");

    // Register send and receive callback functions
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));

    // Configure the peer device
    esp_now_peer_info_t peer_info = {};
    memcpy(peer_info.peer_addr, gun_mac, 6);
    peer_info.channel = 0; // Set channel to 0 (use the current channel)
    peer_info.encrypt = false;

    // Add peer
    ESP_ERROR_CHECK(esp_now_add_peer(&peer_info));
    ESP_LOGI(ESPNOW_TAG, "Peer added: " MACSTR, MAC2STR(gun_mac));
}

void get_own_mac_address(uint8_t *mac)
{
    // Get MAC address for the Wi-Fi STA interface
    esp_err_t result = esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);

    if (result == ESP_OK)
    {
        ESP_LOGI(ESPNOW_TAG, "Device MAC Address: " MACSTR, MAC2STR(mac));
    }
    else
    {
        ESP_LOGE(ESPNOW_TAG, "Failed to get MAC address, error code: %d", result);
    }
}

// The main send function
void send_message(const char *message)
{
    espnow_data_t data;
    memset(&data, 0, sizeof(data));                      // Clear the data structure
    snprintf(data.msg, sizeof(data.msg), "%s", message); // Copy the message to the data structure

    // Send the message to the peer MAC address
    esp_err_t result = esp_now_send(gun_mac, (uint8_t *)&data, sizeof(data));

    if (result == ESP_OK)
    {
        ESP_LOGI(ESPNOW_TAG, "Message sent successfully: %s", data.msg);
    }
    else
    {
        ESP_LOGE(ESPNOW_TAG, "Failed to send message, error code: %d", result);
    }
}

void send_pairing_message() {
    espnow_data_t data;
    memset(&data, 0, sizeof(data));
    snprintf(data.msg, sizeof(data.msg), "Vest Ready");

    esp_err_t result = esp_now_send(gun_mac, (uint8_t *)&data, sizeof(data));
    if (result == ESP_OK) {
        ESP_LOGI(ESPNOW_TAG, "Pairing message sent: %s", data.msg);
        paring = false;
    } else {
        ESP_LOGE(ESPNOW_TAG, "Failed to send pairing message, error code: %d", result);
    }
}

void setupESPnow()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Wi-Fi and ESP-NOW
    wifi_init();
    espnow_init();
    send_pairing_message();
}



