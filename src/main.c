#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Define the GPIO pin where the LED is connected
#define BLINK_GPIO GPIO_NUM_38 // Typically GPIO 2 is connected to an onboard LED on many ESP32 boards.

void app_main(void)
{
    // Configure the GPIO pin gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        // Turn the LED on
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second

        // Turn the LED off
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1 second
    }
}
