#include "Blaster.h"
#include "IRReceiver.h"
#include "IRTransmitter.h"
#include "ESPnow.h"
#include "esp_log.h"
#include <string>
#include "GPIOHelper.h"
#include "RGB_LED.h"

#define TAG "LazerBlaster"

#define TX_PIN GPIO_NUM_17
#define RX_PIN GPIO_NUM_18
#define IR_RESOLUTION_HZ 1000000

// Define GPIO pins for the buttons
#define PLUS_LIFE_BUTTON GPIO_NUM_35
#define MINUS_LIFE_BUTTON GPIO_NUM_36
#define CHANGE_TEAM_BUTTON GPIO_NUM_48
#define MOTOR1 GPIO_NUM_12

int Blaster::health;
uint8_t Blaster::teamAddress;
uint8_t Blaster::playerAddress;
RGB_LED Blaster::rgbLed = RGB_LED(GPIO_NUM_9, GPIO_NUM_3, GPIO_NUM_8);

uint8_t teams[6] = {0, 1, 2, 3, 4, 5};

void Blaster::setup()
{
    IRTransmitter::setup(TX_PIN, IR_RESOLUTION_HZ);

    rgbLed = RGB_LED(GPIO_NUM_9, GPIO_NUM_3, GPIO_NUM_8);

    // Intitialize Motor Pin
    GPIOHelper::initializePinAsOutput(MOTOR1);

    // Start receiving on another thread
    xTaskCreate(&Blaster::startReceiverTask, "StartReceivingTask", 4096, NULL, 5, NULL);
}

// Static function to serve as the task entry point
void Blaster::startReceiverTask(void *pvParameters)
{
    IRReceiver::setup(RX_PIN, IR_RESOLUTION_HZ, [](uint16_t address, uint16_t command)
                      { return onCommandReceived(address, command); });

    IRReceiver::startReceiving();
}

int Blaster::takeDamage(int damage)
{
    for(int x = 0; x < 19; x++){
        ESP_LOGI(TAG, "player tooked %d damage", damage);
    }
    health = getLife() - damage;
    if (health < 0) {
        health = 0;
    }
    setLife(health);

    GPIOHelper::setPinsHighForDuration(MOTOR1, damage * 500);
    ESP_LOGI(TAG, "Player: %04X Has Taken Damage, Current Health is: %d", getTeam(), getLife());
    if (health <= 0)
    {
        // Send death message to vest 
        deathSequence();
    }
    GPIOHelper::setPinsHighForDuration(MOTOR1, 1000);

    char message[50];
    sprintf(message, "Damage %d", health); // When damage is taken, seend new health
    blaster_send_message(message);

    return health;
}

void Blaster::fire(uint8_t gunType)
{
    // Concatenate the team address with the player address
    uint16_t address = (getTeam() << 8) | getLife();
    
    // Add a tag to the command to indicate damage (this is done to prevent the mac address from registering as damage)
    uint16_t command = (0x02 << 8) | gunType; // 0x02 indicates this is a damage command

    IRTransmitter::transmit(address, command); // Send damage command
    ESP_LOGI(TAG, "Fire Method - Address: %04X, Command: %04X", address, command);
}


bool Blaster::onCommandReceived(uint16_t address, uint16_t command)
{
    ESP_LOGI(TAG, "Callback called Address: %04X Command: %04X", address, command);

    // If team is my team do nothing
    if ((address >> 8) == getTeam())
    {
        ESP_LOGI(TAG, "Lazer Came From My Team: %04X No damage taken", address >> 8);
    }
    else if((address) <= 6)
    {
        ESP_LOGI(TAG, "Lazer Came From Opposite Team: %04X Damage taken", address >> 8);
        takeDamage(command);
    }
    return true;
}

void Blaster::deathSequence()
{

   
    while (true)
    {
        ESP_LOGI(TAG, "Player %04X has been killed", playerAddress); 
    
        GPIOHelper::setPinsHighForDuration(MOTOR1, 10000);
        ESP_LOGI(TAG, "Player %04X has been killed", playerAddress);
        vTaskDelay(5000);
    }
}

uint8_t Blaster::getPlayerAddr()
{
    return playerAddress;
}

uint8_t Blaster::getTeamAddr()
{
    return teamAddress;
}

void Blaster::sendMacAddressIR()
{
    // Check if the device is in pairing mode
    if (paring)
    {
        // Breaks the MAC address into 16-bit segments
        uint16_t part1 = (gun_mac[0] << 8) | gun_mac[1];
        uint16_t part2 = (gun_mac[2] << 8) | gun_mac[3];
        uint16_t part3 = (gun_mac[4] << 8) | gun_mac[5];

        // Log the full MAC address and each part to be transmitted
        ESP_LOGI(TAG, "Full MAC address: %02X:%02X:%02X:%02X:%02X:%02X",
                 gun_mac[0], gun_mac[1], gun_mac[2], gun_mac[3], gun_mac[4], gun_mac[5]);

        ESP_LOGI(TAG, "Sending MAC segments - Part1: 0x%04X, Part2: 0x%04X, Part3: 0x%04X",
                 part1, part2, part3);

        // Sends the MAC address parts over IR
        IRTransmitter::transmit(part1, 0x01);
        vTaskDelay(pdMS_TO_TICKS(100));
        IRTransmitter::transmit(part2, 0x01);
        vTaskDelay(pdMS_TO_TICKS(100));
        IRTransmitter::transmit(part3, 0x01);

        ESP_LOGI(TAG, "MAC address sent through IR for pairing.");
    }
}

void Blaster::setTeamColor(uint8_t team)
{
    switch (team)
    {
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