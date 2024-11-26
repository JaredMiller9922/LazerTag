#ifndef GAME_MODE_LOGIC_H
#define GAME_MODE_LOGIC_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Blaster.h"
#include "esp_log.h"
#include "GPIOHelper.h"
#include "ESPnow.h"

class GameModeLogic {
public:
    // Main game loop
    static void run();

private:
    // Helper methods for specific tasks
    static void handleFiring();
    static void handleReloading();
    static void handleWeaponSelection();
    static void handleFlashlight();
    static void updateClipLED(uint8_t clipLevel, uint8_t clipCapacity);
    static void indicateFireReady(uint16_t fireRate);
    static void initialize();
    static void reload(uint16_t reloadTime);
    static void selectGun(uint8_t damage, gpio_num_t ledPin, uint8_t capacity, uint16_t rate, uint16_t reloadTime);
    // Member variables
    static uint8_t prevTriggerButtonState;
    static uint8_t prevReloadButtonState;
    static uint8_t prevFlashlightButtonState;
    static bool flashlightOn;
    // Gun information 
    static uint8_t gunDamage;
    static uint8_t clipCapacity;
    static uint16_t fireRate;
    static uint16_t reloadTimer;
    static uint8_t clipLevel;
    // Private helper to initialize a pin
    void initializePin(gpio_num_t pin, bool isOutput);
};

#endif // GAME_MODE_LOGIC_H
