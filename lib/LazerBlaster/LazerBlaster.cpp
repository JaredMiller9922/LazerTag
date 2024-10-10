#include "LazerBlaster.h"
#include "IRReceiver.h"
#include "IRTransmitter.h"
#include "esp_log.h"

#define TX_PIN GPIO_NUM_17 
#define RX_PIN GPIO_NUM_18
#define IR_RESOLUTION_HZ 1000000

LazerBlaster::LazerBlaster(uint8_t teamAddr, uint8_t playerAddr, int startingHealth) :
    teamAddress(teamAddr), playerAddress(playerAddr), health(startingHealth),
    transmitter(TX_PIN, IR_RESOLUTION_HZ),
    receiver(RX_PIN, IR_RESOLUTION_HZ)
    {
        xTaskCreate(&LazerBlaster::startReceiverTask, "StartReceivingTask", 4096, this, 5, NULL);
    }

// Static function to serve as the task entry point
void LazerBlaster::startReceiverTask(void* pvParameters) {
    LazerBlaster* blaster = static_cast<LazerBlaster*>(pvParameters);
    blaster->receiver.startReceiving();
}

int LazerBlaster::takeDamage(int damage){
    health -= damage;
    return health;
}

void LazerBlaster::fire(){
    // Concatenate the team address with the player address
    uint16_t address = (teamAddress << 8) | playerAddress;
    transmitter.transmit(address, 0x01); // For now only one gun type
    ESP_LOGI("LazerBlaster", "End of Fire Method");
}

void LazerBlaster::onCommandReceived(uint16_t command, uint16_t address){
    ESP_LOGI("LazerBlaster", "Command: %02X Address: %02X", command, address);
}
