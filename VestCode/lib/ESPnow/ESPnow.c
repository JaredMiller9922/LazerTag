
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
uint8_t currentLife = 0;
uint8_t team = 0;

// Paring 
// in this state the gun will send out its ir encoded mac, and also await to recieve data through ir that contains its own mac along with a vest mac then init espnow
bool paring = true;

// Define your own MAC address array
uint8_t vest_mac[6] = {0};
uint8_t gun_mac[6];

// Data structure to hold the message to send
typedef struct
{
    char msg[32];
} espnow_data_t;

// Callback function to handle the received data
void vest_espnow_recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    ESP_LOGI(ESPNOW_TAG, "Received message from: " MACSTR, MAC2STR(info->src_addr));
    ESP_LOGI(ESPNOW_TAG, "Received data: %.*s", len, data);
    
    // Check if the message is a damage update with new health value
    if (strncmp((const char *)data, "Damage", 6) == 0)
    {
        int newHealth;

        // Parse the new health from the Damage message
        if (sscanf((const char *)data, "Damage %d", &newHealth) == 1)
        {
            currentLife = (uint8_t)newHealth;
            ESP_LOGI(ESPNOW_TAG, "Damage received - New Health: %d", currentLife);
        }
        else
        {
            ESP_LOGE(ESPNOW_TAG, "Failed to parse damage message");
        }
    }
}

void blaster_espnow_recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    ESP_LOGI(ESPNOW_TAG, "Received message from: " MACSTR, MAC2STR(info->src_addr));
    ESP_LOGI(ESPNOW_TAG, "Received data: %.*s", len, data);

    // Check if the message is a pairing message from the vest
    if (strncmp((const char *)data, "Vest Ready", len) == 0 && paring)
    {
         // Store the vest's MAC address from the received info
        memcpy(vest_mac, info->src_addr, ESP_NOW_ETH_ALEN);

        // Add the vest as a peer using the received MAC address
        esp_now_peer_info_t peer_info = {};
        memcpy(peer_info.peer_addr, vest_mac, ESP_NOW_ETH_ALEN);
        peer_info.channel = 0; // Use the current channel
        peer_info.encrypt = false;

        // Add the peer
        if (esp_now_add_peer(&peer_info) == ESP_OK)
        {
            ESP_LOGI(ESPNOW_TAG, "Vest paired successfully: " MACSTR, MAC2STR(info->src_addr));
            paring = false; // Update the pairing state
        }
        else
        {
            ESP_LOGE(ESPNOW_TAG, "Failed to pair with vest");
        }
    }
    // Check if the message is a setup message with life and team values
    else if (strncmp((const char *)data, "Setup", 5) == 0)
    {
        int receivedTeam, receivedLife;

        // Parse team and life
        if (sscanf((const char *)data, "Setup %d %d", &receivedTeam, &receivedLife) == 2)
        {
            team = (uint8_t)receivedTeam;
            currentLife = (uint8_t)receivedLife;
            ESP_LOGI(ESPNOW_TAG, "Setup received - Team: %d, Life: %d", team, currentLife);
        }
        else
        {
            ESP_LOGE(ESPNOW_TAG, "Failed to parse setup message");
        }
    }
    // Check if the message is a damage update with new health value
    else if (strncmp((const char *)data, "Damage", 6) == 0)
    {
        int newHealth;

        // Parse the new health from the Damage message
        if (sscanf((const char *)data, "Damage %d", &newHealth) == 1)
        {
            currentLife = (uint8_t)newHealth;
            ESP_LOGI(ESPNOW_TAG, "Damage received - New Health: %d", currentLife);
        }
        else
        {
            ESP_LOGE(ESPNOW_TAG, "Failed to parse damage message");
        }
    }
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

void vest_espnow_init()
{
    ESP_ERROR_CHECK(esp_now_init());
    ESP_LOGI(ESPNOW_TAG, "Vest ESP-NOW initialized");

    // Register send and receive callback functions
    ESP_ERROR_CHECK(esp_now_register_recv_cb(vest_espnow_recv_cb));
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

void blaster_espnow_init()
{
    ESP_ERROR_CHECK(esp_now_init());
    ESP_LOGI(ESPNOW_TAG, "Blaster ESP-NOW initialized");

    // Register send and receive callback functions
    ESP_ERROR_CHECK(esp_now_register_recv_cb(blaster_espnow_recv_cb));
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));
}

void get_gun_mac_address(uint8_t *mac)
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
void vest_send_message(const char *message)
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

void blaster_send_message(const char *message)
{
    // TODO: may want to combine this method witht he vest method
    // Check if the vest's MAC address has been set (not all zeros)
    if (memcmp(vest_mac, "\0\0\0\0\0\0", ESP_NOW_ETH_ALEN) == 0)
    {
        ESP_LOGW(ESPNOW_TAG, "Cannot send message, vest not paired yet.");
        return;
    }

    espnow_data_t data;
    memset(&data, 0, sizeof(data));                      // Clear the data structure
    snprintf(data.msg, sizeof(data.msg), "%s", message); // Copy the message to the data structure

    // Send the message to the peer MAC address
    esp_err_t result = esp_now_send(vest_mac, (uint8_t *)&data, sizeof(data));

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

void vest_setupESPnow()
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
    vest_espnow_init();
    send_pairing_message();
}

void blaster_setupESPnow()
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
    blaster_espnow_init();
    get_gun_mac_address(gun_mac);
}

uint8_t getLife(){
    return currentLife;
}

void setLife(uint8_t newLife){
    currentLife = newLife;
}

uint8_t getTeam(){
    return team;
}

void setTeam(uint8_t newTeam){
    team = newTeam;
}