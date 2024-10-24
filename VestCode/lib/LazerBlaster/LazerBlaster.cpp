#include "LazerBlaster.h"
#include "IRReceiver.h"
#include "IRTransmitter.h"
#include "ESPnow.h"
#include "esp_log.h"

#define TAG "LazerBlaster"

#define TX_PIN GPIO_NUM_17 
#define RX_PIN GPIO_NUM_18
#define IR_RESOLUTION_HZ 1000000

LazerBlaster::LazerBlaster(uint8_t teamAddr, uint8_t playerAddr, int startingHealth) :
    teamAddress(teamAddr), playerAddress(playerAddr), health(startingHealth),
    transmitter(TX_PIN, IR_RESOLUTION_HZ),
    // Register the callback so that when things are received our method can be called
    receiver(RX_PIN, IR_RESOLUTION_HZ, [this](uint16_t address, uint16_t command) {
        return onCommandReceived(address, command);
    })
    {
        xTaskCreate(&LazerBlaster::startReceiverTask, "StartReceivingTask", 4096, this, 5, NULL);
    }
// Static function to serve as the task entry point
void LazerBlaster::startReceiverTask(void *pvParameters)
{
    LazerBlaster *blaster = static_cast<LazerBlaster *>(pvParameters);
    blaster->receiver.startReceiving();
}

int LazerBlaster::takeDamage(int damage)
{
    health -= damage;
    ESP_LOGI(TAG, "Player: %04X Has Taken Damage, Current Health is: %d", playerAddress, health);
    if (health <= 0) {
        deathSequence();
    }
    return health;
}

void LazerBlaster::fire()
{
    // Concatenate the team address with the player address
    uint16_t address = (teamAddress << 8) | playerAddress;
    transmitter.transmit(address, 0x01); // For now only one gun type
    ESP_LOGI(TAG, "End of Fire Method");
}

bool LazerBlaster::onCommandReceived(uint16_t address, uint16_t command){
    static uint16_t received_mac_parts[3];
    static int partIndex = 0;

    if (paring) {
        // Store the received MAC address parts in sequence
        received_mac_parts[partIndex] = address;
        partIndex++;

        if (partIndex >= 3) {
            // Once all parts are received, reconstruct the MAC address
            gun_mac[0] = (received_mac_parts[0] >> 8) & 0xFF;
            gun_mac[1] = received_mac_parts[0] & 0xFF;
            gun_mac[2] = (received_mac_parts[1] >> 8) & 0xFF;
            gun_mac[3] = received_mac_parts[1] & 0xFF;
            gun_mac[4] = (received_mac_parts[2] >> 8) & 0xFF;
            gun_mac[5] = received_mac_parts[2] & 0xFF;

            ESP_LOGI(TAG, "Received Gun MAC Address: " MACSTR, MAC2STR(gun_mac));
            receivedGunAddress = true;
            // Proceed with pairing
            pairWithGun();

            // Reset part index for future pairings
            partIndex = 0;
        }
        return true;
    } else {
        ESP_LOGI(TAG, "Callback called Address: %04X Command: %04X", address, command);
        // If team is my team do nothing
        if ((address >> 8) == teamAddress) {
            ESP_LOGI(TAG, "Lazer Came From My Team: %04X No damage taken", address);
        }
        else {
         ESP_LOGI(TAG, "Lazer Came From Opposite Team: %04X Damage taken", address);
            takeDamage(1);
        }
        return true;
    }
}

void LazerBlaster::deathSequence(){
    ESP_LOGI(TAG, "Player %04X has been killed", playerAddress);
}

uint8_t LazerBlaster::getPlayerAddr(){
    return playerAddress;
}

uint8_t LazerBlaster::getTeamAddr(){
    return teamAddress;
}

void LazerBlaster::pairWithGun(){
   
    while(!receivedGunAddress){
        vTaskDelay(10);
    }
    setupESPnow();
    ESP_LOGI(TAG, "recieved gun address");
    ESP_LOGI(ESPNOW_TAG, "gun address: " MACSTR, MAC2STR(gun_mac));
    while(paring){
        vTaskDelay(10);
    }
    ESP_LOGI(TAG, "vest pared with gun");
}