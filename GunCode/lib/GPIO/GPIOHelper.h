#ifndef GPIO_H
#define GPIO_H

#include "driver/gpio.h"

class GPIOHelper {
    public:
        static bool initializePinButton(gpio_num_t gpioNum);
        static bool initializePinLED(gpio_num_t gpioNum);
};

#endif