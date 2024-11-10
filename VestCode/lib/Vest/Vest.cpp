#include "Vest.h"
#include "IRReceiver.h"
#include "IRTransmitter.h"
#include "ESPnow.h"
#include "esp_log.h"
#include <string>
#include "GPIOHelper.h"
#include "RGB_LED.h"


#define TAG "Vest"

#define TX_PIN GPIO_NUM_17 
#define RX_PIN GPIO_NUM_18
#define IR_RESOLUTION_HZ 1000000

// Define GPIO pins for the buttons
#define PLUS_LIFE_BUTTON GPIO_NUM_35
#define MINUS_LIFE_BUTTON GPIO_NUM_36
#define CHANGE_TEAM_BUTTON GPIO_NUM_48
#define MOTOR1 GPIO_NUM_38
#define MOTOR2 GPIO_NUM_37


uint8_t teams[6] = {0, 1, 2, 3, 4, 5};

void Vest::setup() 
{
    rgbLed = RGB_LED(GPIO_NUM_3, GPIO_NUM_8, GPIO_NUM_9);

    // Intitialize Motor Pin
    GPIOHelper::initializePinAsOutput(MOTOR1);
    GPIOHelper::initializePinAsOutput(MOTOR2);

    // Start receiving on another thread
    xTaskCreate(&Vest::startReceiverTask, "StartReceivingTask", 4096, NULL, 5, NULL);
}

// Static function to serve as the task entry point
void Vest::startReceiverTask(void *pvParameters)
{   
    IRReceiver::setup(RX_PIN, IR_RESOLUTION_HZ, [](uint16_t address, uint16_t command) {
        return onCommandReceived(address, command);
    });

    IRReceiver::startReceiving();
}

int Vest::takeDamage(int damage)
{
    health = getLife() - damage;
    setLife(health);
    
    ESP_LOGI(TAG, "Player: %04X Has Taken Damage, Current Health is: %d", getTeam(), getLife());
    if (health <= 0) {
        deathSequence();
    }
    GPIOHelper::setPinsHighForDuration(MOTOR1, MOTOR2, 1000);


    char message[50];
    sprintf(message, "Damage %d", health); // When damage is taken, seend new health
    vest_send_message(message);

    return health;
}



bool Vest::onCommandReceived(uint16_t address, uint16_t command){
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
        if ((address >> 8) == getTeam()) {
            ESP_LOGI(TAG, "Lazer Came From My Team: %04X No damage taken", address>>8);
        }
        else {
         ESP_LOGI(TAG, "Lazer Came From Opposite Team: %04X Damage taken", address>>8);
         ESP_LOGI(TAG, "My team is: %04X", getTeam());

            takeDamage(command);
        }
        return true;
    }
}

void Vest::deathSequence(){
    ESP_LOGI(TAG, "Player %04X has been killed", playerAddress);
}

uint8_t Vest::getPlayerAddr(){
    return playerAddress;
}

uint8_t Vest::getTeamAddr(){
    return teamAddress;
}

void Vest::pairWithGun(){
   
    vest_setupESPnow();
    ESP_LOGI(TAG, "recieved gun address");
    ESP_LOGI(ESPNOW_TAG, "gun address: " MACSTR, MAC2STR(gun_mac));
    while(paring){
        vTaskDelay(10);
    }
    ESP_LOGI(TAG, "vest pared with gun");
}

void Vest::gameSetUp(){
    bool setupcomplete = false;
    uint8_t tempLife = 1;
    uint8_t tempTeam = 0;
    uint8_t tempTeamIndex = 0;
    int inactivityTimer = 0;
    int inactivityTimerLimit = 100; // about 20 seconds with a print statment and a vtaskdelay(10) 200

    GPIOHelper::initializePinButton(MINUS_LIFE_BUTTON);
    GPIOHelper::initializePinButton(CHANGE_TEAM_BUTTON);
    GPIOHelper::initializePinButton(PLUS_LIFE_BUTTON);

    setTeamColor(tempTeam);
    
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
                ESP_LOGI(TAG, "Current health is already at 1.");
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
            setTeamColor(tempTeam);
            vTaskDelay(100 / portTICK_PERIOD_MS); // Debounce delay
        }

        inactivityTimer++;
        // ESP_LOGI(TAG, "Timer: %d", inactivityTimer);

        if (inactivityTimer >= inactivityTimerLimit){
            setupcomplete = true;
        }

        vTaskDelay(5);
    }
    ESP_LOGI(TAG, "Team: %d", tempTeam);
    ESP_LOGI(TAG, "Life: %d", tempLife);

    setLife(tempLife);
    setTeam(tempTeam);

    ESP_LOGI("lazerblaster-setup", "life: %d, Team %d", getLife(), getTeam());

    char message[50];
    sprintf(message, "Setup %d %d", tempTeam, tempLife); // Format the variables into the string

    ESP_LOGI(TAG, "Sending Message: %s", message);

    // Send data through espnow
    vest_send_message(message);
}

void Vest::setTeamColor(uint8_t team) {
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


bool Vest::getParingStatus(){
    return paring;
}
