#include "LazerBlaster.h"
#include "IRReceiver.h"
#include "IRTransmitter.h"
#include "ESPnow.h"
#include "esp_log.h"
#include "RGB_LED.h"

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
    }),
    rgbLed(GPIO_NUM_3, GPIO_NUM_8, GPIO_NUM_9) 
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
    health = getLife() - damage;
    setLife(health);

    ESP_LOGI(TAG, "Player: %04X Has Taken Damage, Current Health is: %d", getTeam(), getLife());
    if (health <= 0) {
        deathSequence();
    }

    char message[50];
    sprintf(message, "Damage %d", health); // When damage is taken, seend new health
    send_message(message);

    return health;
}

void LazerBlaster::fire(uint8_t gunType)
{
    // Concatenate the team address with the player address
    uint16_t address = (getTeam() << 8) | getLife();
    transmitter.transmit(address, gunType); // For now only one gun type
    ESP_LOGI(TAG, "End of Fire Method");
}

bool LazerBlaster::onCommandReceived(uint16_t address, uint16_t command){
    ESP_LOGI(TAG, "Callback called Address: %04X Command: %04X", address, command);

    uint16_t testValue = address >> 8;
    
    // If team is my team do nothing
    if ((address >> 8) == getTeam()) {
        ESP_LOGI(TAG, "Lazer Came From My Team: %04X No damage taken", address>>8);
    }
    else {
        ESP_LOGI(TAG, "Lazer Came From Opposite Team: %04X Damage taken", address>>8);
        takeDamage(command);
    }
    return true;
}

void LazerBlaster::deathSequence(){
    ESP_LOGI(TAG, "Player %04X has been killed", getTeam());
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

void LazerBlaster::setTeamColor(uint8_t team) {
    switch (team) {
        case 0:
            rgbLed.setRed();
            break;
        case 1:
            rgbLed.setGreen();
            break;
        case 2:
            rgbLed.setBlue();
            break;
        case 3:
            rgbLed.setYellow(); // Red + Green
            break;
        case 4:
            rgbLed.setMagenta(); // Red + Blue
            break;
        default:
            rgbLed.setWhite(); // Default to White This is Rouge
            break;
    }
}



