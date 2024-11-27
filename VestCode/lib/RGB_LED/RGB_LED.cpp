#include "RGB_LED.h"

// Constructor to initialize RGB LED pins
RGB_LED::RGB_LED(gpio_num_t redPin, gpio_num_t greenPin, gpio_num_t bluePin)
    : redPin(redPin), greenPin(greenPin), bluePin(bluePin)
{
    // Initialize each pin as an output
    initializePin(redPin);
    initializePin(greenPin);
    initializePin(bluePin);
}

// Second constructor for special use case
RGB_LED::RGB_LED(gpio_num_t redPin, gpio_num_t greenPin, gpio_num_t bluePin, bool isLifeIndicator)
    : redPin(redPin), greenPin(greenPin), bluePin(bluePin)
{
    // Initialize each pin as an output
    initializePin(redPin);
    initializePin(greenPin);
    initializePin(bluePin);
    if (isLifeIndicator == true){
        setWhite();
    }


    
}

// Method to initialize a GPIO pin as output
void RGB_LED::initializePin(gpio_num_t pin) {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

// Method to set custom RGB color
void RGB_LED::setColor(uint8_t red, uint8_t green, uint8_t blue) {
    setPin(redPin, red > 0);
    setPin(greenPin, green > 0);
    setPin(bluePin, blue > 0);
}

// Method to set individual pin level
void RGB_LED::setPin(gpio_num_t pin, bool level) {
    gpio_set_level(pin, level ? 1 : 0);
}

// Predefined colors
void RGB_LED::setWhite() {
    setColor(1, 1, 1);
}

void RGB_LED::setRed() {
    setColor(1, 0, 0);
}

void RGB_LED::setGreen() {
    setColor(0, 1, 0);
}

void RGB_LED::setBlue() {
    setColor(0, 0, 1);
}

void RGB_LED::setYellow() { // Red + Green
    setColor(1, 1, 0);
}

void RGB_LED::setMagenta() { // Red + Blue
    setColor(1, 0, 1);
}

void RGB_LED::setCyan() { // Green + Blue
    setColor(0, 1, 1);
}

void RGB_LED::turnOff() {
    setColor(0, 0, 0);
}
