#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LazerBlaster.h"
#include "esp_log.h"
#include "GPIOHelper.h"

#define TAG "Main"

#define TX_PIN GPIO_NUM_17 
#define RX_PIN GPIO_NUM_18
#define IR_RESOLUTION_HZ 1000000

#define RED_TEAM_ADDR 0x01
#define BLUE_TEAM_ADDR 0x02

#define PLAYER1_ADDR 0x01
#define PLAYER2_ADDR 0x02

extern "C" void app_main(void) {
    // Create a Lazer Blaster
    LazerBlaster player(BLUE_TEAM_ADDR, PLAYER1_ADDR, 5);
    // LazerBlaster player2(BLUE_TEAM_ADDR, PLAYER2_ADDR, 5);


    ESP_LOGI(TAG, "Player: %04X Team: %04X", player.getPlayerAddr(), player.getTeamAddr());
    // ESP_LOGI(TAG, "Player: %04X Team: %04X", player2.getPlayerAddr(), player2.getTeamAddr());

    while(player.getParingStatus()){
        vTaskDelay(10);
    }
    player.gameSetUp();
    // Infinite Program Logic
    while(true) {

        vTaskDelay(10);

    }
    
    ESP_LOGI("main", "Program Terminated");
}

