#include "GPIOHelper.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

bool GPIOHelper::initializePinButton(gpio_num_t gpioNum) {
    // Configure the pin as input
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE; // Trigger on falling edge (button press)
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << gpioNum); 
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; 
    gpio_config(&io_conf);

    // Implement debouncing logic
    const int debounceDelay = 50; // 50ms debounce delay
    static int lastButtonState = 1; // Assume button is not pressed at start
    int buttonState = gpio_get_level(gpioNum);

    // Check if the button state has changed
    if (buttonState != lastButtonState) {
        // Wait for the debounce delay
        vTaskDelay(pdMS_TO_TICKS(debounceDelay));

        // Read the button state again
        buttonState = gpio_get_level(gpioNum);

        // If button state is stable and has changed
        if (buttonState == 0 && lastButtonState == 1) {
            // Button press detected
            lastButtonState = buttonState;
            return true; // Button pressed
        }
    }

    // Update the last button state
    lastButtonState = buttonState;

    return false; // No valid button press detected
}

bool GPIOHelper::initializePinLED(gpio_num_t gpioNum) {
    // Configure the pin as output
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;   // No interrupt needed for LED
    io_conf.mode = GPIO_MODE_OUTPUT;         // Set as output mode
    io_conf.pin_bit_mask = (1ULL << gpioNum); // Set the bit for the LED GPIO pin
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // No pull-down needed
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;   // No pull-up needed
    esp_err_t ret = gpio_config(&io_conf);

    // Check if the configuration was successful
    if (ret == ESP_OK) {
        // Initialization successful
        return true;
    } else {
        // Initialization failed
        return false;
    }
}
