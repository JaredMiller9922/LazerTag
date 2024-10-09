#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "IRReceiver.h"
#include "IRTransmitter.h"
#include "esp_log.h"

#define TX_PIN GPIO_NUM_17 
#define RX_PIN GPIO_NUM_18
#define IR_RESOLUTION_HZ 1000000

#define RED_TEAM_COMMAND 0x01
#define BLUE_TEAM_COMMAND 0x02

extern "C" void app_main(void) {
    // Create a Receiver
    IRReceiver receiver(RX_PIN, IR_RESOLUTION_HZ);
    ESP_LOGI("app_main", "Receiver initialized");

    // Create a Transmitter
    IRTransmitter transmitter(TX_PIN, IR_RESOLUTION_HZ);
    ESP_LOGI("app_main", "Transmitter initialized");

    while(true){

        ESP_LOGI("app_main", "Waiting for signal");
        receiver.startReceiving();
        ESP_LOGI("app_main", "Transmitting signal");
        transmitter.transmit(0x00, RED_TEAM_COMMAND);

        // Add a delay to prevent rapid transmissions
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

