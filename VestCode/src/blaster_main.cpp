#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Blaster.h"
#include "esp_log.h"
#include "GPIOHelper.h"
#include "ESPnow.h"
#include "GameModeLogic.h"
#define TAG "Main"

#define TX_PIN GPIO_NUM_17 
#define RX_PIN GPIO_NUM_18
#define TRIGGER_PIN GPIO_NUM_36


#define IR_RESOLUTION_HZ 1000000

#define RED_TEAM_ADDR 0x01
#define BLUE_TEAM_ADDR 0x02

#define PLAYER1_ADDR 0x01
#define PLAYER2_ADDR 0x02

extern "C" void app_main(void) {
    // Create a Lazer Blaster
    Blaster::setup();
    GPIOHelper::initializePinButton(TRIGGER_PIN);
    // LazerBlaster player(BLUE_TEAM_ADDR, PLAYER1_ADDR, 5);
    // LazerBlaster player2(BLUE_TEAM_ADDR, PLAYER2_ADDR, 5);

    // Begin paring process
    blaster_setupESPnow();
    while(paring){
        //ESP_LOGI(TAG, "Pairing status: %d", paring);
        vTaskDelay(10);
        // if trigger pressed
        if (gpio_get_level(TRIGGER_PIN) == 0){
        // fire(ownMac)
        Blaster::sendMacAddressIR();

        } 
    }
    ESP_LOGI(TAG, "Gun: pared with vest");

    bool setUpComplete = false;
    setLife(0);
    while (!setUpComplete){
        ESP_LOGI(TAG, "getLife: %d", getLife());
        if (getLife() > 0){
            ESP_LOGI(TAG, "getTeam: %d", getTeam());
            setUpComplete = true;
        }
        vTaskDelay(100);
        Blaster::setTeamColor(getTeam());
    }

    GameModeLogic::run();
 
    
    ESP_LOGI("gun main", "Program Terminated");
}