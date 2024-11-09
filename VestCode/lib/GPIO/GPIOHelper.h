#ifndef GPIO_H
#define GPIO_H

#include "driver/gpio.h"

class GPIOHelper {
    public:
        static bool initializePinButton(gpio_num_t gpioNum);
        static bool initializePinLED(gpio_num_t gpioNum);
        static bool initializePinAsOutput(gpio_num_t gpioNum);
        static void setPinsHighTask(void *param);
        static void setPinsHighForDuration(gpio_num_t motor1, gpio_num_t motor2, int durationMs);
};

#endif