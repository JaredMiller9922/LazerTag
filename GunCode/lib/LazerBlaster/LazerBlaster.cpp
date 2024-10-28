#include "LazerBlaster.h"
#include "IRReceiver.h"
#include "IRTransmitter.h"
#include "ESPnow.h"
#include "esp_log.h"

#define TAG "LazerBlaster"

#define TX_PIN GPIO_NUM_17 
#define RX_PIN GPIO_NUM_18
#define IR_RESOLUTION_HZ 1000000

uint8_t life = 1;
uint8_t teams[6] = {0, 1, 2, 3, 4, 5};
uint8_t myTeam;

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

void LazerBlaster::deathSequence(){
    ESP_LOGI(TAG, "Player %04X has been killed", playerAddress);
}

uint8_t LazerBlaster::getPlayerAddr(){
    return playerAddress;
}

uint8_t LazerBlaster::getTeamAddr(){
    return teamAddress;
}

void LazerBlaster::sendMacAddressIR() {
    // Check if the device is in pairing mode
    if (paring) {
        // Breaks the MAC address into 16-bit segments
        uint16_t part1 = (own_mac[0] << 8) | own_mac[1];
        uint16_t part2 = (own_mac[2] << 8) | own_mac[3];
        uint16_t part3 = (own_mac[4] << 8) | own_mac[5];

        // Sends the MAC address parts over IR
        transmitter.transmit(part1, 0x01);
        vTaskDelay(pdMS_TO_TICKS(100));
        transmitter.transmit(part2, 0x01);
        vTaskDelay(pdMS_TO_TICKS(100));
        transmitter.transmit(part3, 0x01);

        ESP_LOGI(TAG, "MAC address sent through IR for pairing.");
    }
}

// Method to get the color associated with a team
std::string LazerBlaster::getTeamColor(uint8_t team) {
    switch (team) {
        case 0: return "Red";
        case 1: return "Green";
        case 2: return "Blue";
        case 3: return "Yellow";
        case 4: return "Orange";
        default: return "Rouge";
    }
}

void LazerBlaster::setLife(uint8_t startLife){
    life = startLife;
    ESP_LOGI(TAG, "%d", life);
}

void LazerBlaster::setTeam(uint8_t team){
    myTeam = team;
    ESP_LOGI(TAG, "%d", myTeam);

}


