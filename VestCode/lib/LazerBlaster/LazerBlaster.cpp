#include "LazerBlaster.h"
#include "IRReceiver.h"
#include "IRTransmitter.h"
#include "ESPnow.h"
#include "esp_log.h"
#include <string>
#include "GPIOHelper.h"

#define TAG "LazerBlaster"

#define TX_PIN GPIO_NUM_17 
#define RX_PIN GPIO_NUM_18
#define IR_RESOLUTION_HZ 1000000

// Define GPIO pins for the buttons
#define PLUS_LIFE_BUTTON GPIO_NUM_35
#define MINUS_LIFE_BUTTON GPIO_NUM_36
#define CHANGE_TEAM_BUTTON GPIO_NUM_48

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
   
    setupESPnow();
    ESP_LOGI(TAG, "recieved gun address");
    ESP_LOGI(ESPNOW_TAG, "gun address: " MACSTR, MAC2STR(gun_mac));
    while(paring){
        vTaskDelay(10);
    }
    ESP_LOGI(TAG, "vest pared with gun");
}

void LazerBlaster::gameSetUp(){
    bool setupcomplete = false;
    uint8_t tempLife = 1;
    uint8_t tempTeam = 0;
    uint8_t tempTeamIndex = 0;
    int inactivityTimer = 0;
    int inactivityTimerLimit = 200; // about 20 seconds with a print statment and a vtaskdelay(10)

    GPIOHelper::initializePinButton(MINUS_LIFE_BUTTON);
    GPIOHelper::initializePinButton(CHANGE_TEAM_BUTTON);
    GPIOHelper::initializePinButton(PLUS_LIFE_BUTTON);

    
    while (!setupcomplete){
        // Check if the Plus Life button is pressed
        if (gpio_get_level(PLUS_LIFE_BUTTON) == 0){
            inactivityTimer = 0;
            tempLife++;
            ESP_LOGI(TAG, "Plus Life button pressed. Current health: %d", tempLife);
            vTaskDelay(100 / portTICK_PERIOD_MS); // Debounce delay
        }
        
        // Check if the Minus Life button is pressed
        if (gpio_get_level(MINUS_LIFE_BUTTON) == 0) {
            inactivityTimer = 0;
            if (tempLife > 1) { // Ensure tempLife doesn't go below one
                tempLife--;
                ESP_LOGI(TAG, "Minus Life button pressed. Current health: %d", tempLife);
            } else {
                ESP_LOGI(TAG, "Current health is already at zero.");
            }
            vTaskDelay(100 / portTICK_PERIOD_MS); // Debounce delay
        }

        // Check if the Change Team button is pressed
        if (gpio_get_level(CHANGE_TEAM_BUTTON) == 0) {
            inactivityTimer = 0;
            // Increment the index and wrap around if it reaches the end of the array
            tempTeamIndex = (tempTeamIndex + 1) % 6;
            tempTeam = teams[tempTeamIndex];
            ESP_LOGI(TAG, "Change Team button pressed. New team: %d", tempTeam);
            vTaskDelay(100 / portTICK_PERIOD_MS); // Debounce delay
        }

        inactivityTimer++;
        vTaskDelay(10);
        ESP_LOGI(TAG, "Timer: %d", inactivityTimer);
        if (inactivityTimer >= inactivityTimerLimit){
            setupcomplete = true;
        }
    }

    ESP_LOGI(TAG, "Team: %d", tempTeam);
    ESP_LOGI(TAG, "Life: %d", tempLife);

    char message[50];
    sprintf(message, "Setup %d %d", tempTeam, tempLife); // Format the variables into the string

    ESP_LOGI(TAG, "Sending Message: %s", message);

    // Send data through espnow
    send_message(message);
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

void updateLife(uint8_t newLife){
    life = newLife;
}

bool LazerBlaster::getParingStatus(){
    return(paring);
}