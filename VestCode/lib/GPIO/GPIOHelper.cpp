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
    // Cast the parameter to gpio_num_t
    gpio_num_t gpioNum = *reinterpret_cast<gpio_num_t *>(param);

    ESP_LOGI("GPIOHelper", "Task started for GPIO pin %d", gpioNum);

    // Set the pin high
    ESP_LOGI("GPIOHelper", "Setting GPIO pin %d HIGH", gpioNum);
    gpio_set_level(gpioNum, 1);

    // Delay for the specified duration
    ESP_LOGI("GPIOHelper", "GPIO pin %d will remain HIGH for 5000 ms", gpioNum);
    vTaskDelay(pdMS_TO_TICKS(5000)); // Delay for 5 seconds

    // Set the pin low
    ESP_LOGI("GPIOHelper", "Setting GPIO pin %d LOW", gpioNum);
    gpio_set_level(gpioNum, 0);

    // Free the dynamically allocated memory
    delete reinterpret_cast<gpio_num_t *>(param);

    // Log task completion
    ESP_LOGI("GPIOHelper", "Task for GPIO pin %d completed. Deleting task.", gpioNum);

    // Delete the task when done
    vTaskDelete(NULL);
}



void GPIOHelper::setPinsHighForDuration(gpio_num_t motor1, int durationMs) {
    ESP_LOGI("gpiohelper", "setPinsHighForDuration called with GPIO pin %d for %d ms", motor1, durationMs);

    // Dynamically allocate memory for the GPIO pin
    gpio_num_t *param = new gpio_num_t(motor1);

    // Create the task, passing the dynamically allocated memory
    BaseType_t result = xTaskCreate(setPinsHighTask, "setPinsHighTask", 2048, param, 1, NULL);
    if (result != pdPASS) {
        ESP_LOGE("gpiohelper", "Failed to create setPinsHighTask");
        delete param; // Clean up if task creation fails
    }
}
