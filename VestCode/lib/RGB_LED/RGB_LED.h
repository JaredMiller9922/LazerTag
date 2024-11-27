#ifndef RGB_LED_H
#define RGB_LED_H

#include "driver/gpio.h" // ESP-IDF GPIO functions

class RGB_LED {
public:
    RGB_LED(gpio_num_t redPin, gpio_num_t greenPin, gpio_num_t bluePin);
    RGB_LED(gpio_num_t redPin, gpio_num_t greenPin, gpio_num_t bluePin, bool isLifeIndicator);

    // Function to set custom RGB values
    void setColor(uint8_t red, uint8_t green, uint8_t blue);

    // Functions for predefined colors
    void setWhite();
    void setRed();
    void setGreen();
    void setBlue();
    void setYellow();
    void setMagenta();
    void setCyan();
    void turnOff();

private:
    gpio_num_t redPin;
    gpio_num_t greenPin;
    gpio_num_t bluePin;

    void initializePin(gpio_num_t pin);
    void setPin(gpio_num_t pin, bool level);
};

#endif // RGB_LED_H
