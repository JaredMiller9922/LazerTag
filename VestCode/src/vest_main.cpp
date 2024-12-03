#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Vest.h"
#include "esp_log.h"
#include "GPIOHelper.h"
#include "ESPnow.h"

#define TAG "Main"

#define TX_PIN GPIO_NUM_17 
#define RX_PIN GPIO_NUM_18
#define IR_RESOLUTION_HZ 1000000

#define RED_TEAM_ADDR 0x01
#define BLUE_TEAM_ADDR 0x02

#define PLAYER1_ADDR 0x01
#define PLAYER2_ADDR 0x02

// Initialize static variables here to allocate space 
int Vest::health;
uint8_t Vest::teamAddress;
uint8_t Vest::playerAddress;


extern "C" void app_main(void) {
    Vest::setup();

    ESP_LOGI(TAG, "Player: %04X Team: %04X", Vest::getPlayerAddr(), Vest::getTeamAddr());
    // ESP_LOGI(TAG, "Player: %04X Team: %04X", player2.getPlayerAddr(), player2.getTeamAddr());

    while(Vest::getParingStatus()){
        vTaskDelay(10);
    }
    Vest::gameSetUp();

    // Infinite Program Logic
    while(true) {

        vTaskDelay(10);
        Vest::setLifeCountLED(getLife());
        if(getLife() <= 0){
            Vest::deathSequence();
        }
    }
    ESP_LOGI("main", "Program Terminated");
}

