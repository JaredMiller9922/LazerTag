#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LazerBlaster.h"
#include "esp_log.h"

#define TX_PIN GPIO_NUM_17 
#define RX_PIN GPIO_NUM_18
#define IR_RESOLUTION_HZ 1000000

#define RED_TEAM_ADDR 0x01
#define BLUE_TEAM_ADDR 0x02

#define PLAYER1_ADDR 0x01
#define PLAYER2_ADDR 0x02

extern "C" void app_main(void) {
    // Create a Lazer Blaster
    LazerBlaster player1(RED_TEAM_ADDR, PLAYER2_ADDR, 5);

    // Have a delay so things aren't so speedy
    while(true) {
        // player1.fire();
        vTaskDelay(500);
    }
    
    ESP_LOGI("main", "Program Terminated");
}

