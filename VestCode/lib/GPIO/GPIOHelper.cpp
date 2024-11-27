#include "GPIOHelper.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

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

bool GPIOHelper::initializePinAsOutput(gpio_num_t gpioNum) {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;     // No interrupt needed
    io_conf.mode = GPIO_MODE_OUTPUT;           // Set as output mode
    io_conf.pin_bit_mask = (1ULL << gpioNum);  // Set the bit for the specified GPIO pin
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // No pull-down needed
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;     // No pull-up needed
    esp_err_t ret = gpio_config(&io_conf);
    gpio_set_level(gpioNum, 0); 

    // Return true if initialization was successful, false otherwise
    return ret == ESP_OK;
}

void GPIOHelper::setPinsHighTask(void *param) {
    gpio_num_t *gpioPins = static_cast<gpio_num_t *>(param);
    gpio_num_t gpioNum1 = gpioPins[0];
    gpio_num_t gpioNum2 = gpioPins[1];
    
    // Set both pins low
    gpio_set_level(gpioNum1, 1); // Set first pin high
    gpio_set_level(gpioNum2, 1); // Set second pin high
    vTaskDelay(pdMS_TO_TICKS(5000)); // Delay for 1 second (non-blocking)

    // Set both pins back to high
    gpio_set_level(gpioNum1, 0); // Set first pin low
    gpio_set_level(gpioNum2, 0); // Set second pin low
    
    delete[] gpioPins; // Free allocated memory
    vTaskDelete(NULL); // Delete this task once done
}

// Method to start the task for setting two pins high for a duration
void GPIOHelper::setPinsHighForDuration(gpio_num_t motor1, int durationMs) {
    ESP_LOGI("gpiohelper", "I got here: setPinsHighForDuration called with GPIO pins %d for %d ms", motor1, durationMs);
    
    int level1 = gpio_get_level(motor1);
    ESP_LOGI("gpiohelper", "Current level of motor1 (GPIO %d): %d", motor1, level1);

    // Allocate an array to hold the GPIO pins
    gpio_num_t *gpioPins = new gpio_num_t[1]{motor1};
    
    // Pass the array as a task parameter
    xTaskCreate(setPinsHighTask, "setPinsHighTask", 1024, gpioPins, 1, NULL);
}