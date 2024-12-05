#include "GameModeLogic.h"

// Tag for logging
#define TAG "GameModeLogic"
#define TRIGGER_PIN GPIO_NUM_36
#define FIREING_INDICATOR_LED GPIO_NUM_15
#define PISTOL_BUTTON GPIO_NUM_47
#define PISTOL_LED GPIO_NUM_19
#define MACHINE_GUN_BUTTON GPIO_NUM_48
#define MACHINE_GUN_LED GPIO_NUM_20
#define RIFLE_BUTTON GPIO_NUM_45
#define RIFLE_LED GPIO_NUM_21
#define ROCKET_BUTTON GPIO_NUM_35
#define ROCKET_LED GPIO_NUM_14
#define RELOAD_BUTTON GPIO_NUM_37
#define CLIP_LEVEL_RED GPIO_NUM_40
#define CLIP_LEVEL_YELLO GPIO_NUM_38
#define CLIP_LEVEL_GREEN GPIO_NUM_39
#define FLASHLIGHT_BUTTON GPIO_NUM_41
#define FLASHLIGHT_LED GPIO_NUM_16
#define READY_TO_FIRE_1 GPIO_NUM_7
#define READY_TO_FIRE_2 GPIO_NUM_6
#define READY_TO_FIRE_3 GPIO_NUM_5
#define READY_TO_FIRE_4 GPIO_NUM_4
// Static member variables
uint8_t GameModeLogic::gunDamage = 1;
uint8_t GameModeLogic::clipCapacity = 10;
uint8_t GameModeLogic::clipLevel = 10;
uint16_t GameModeLogic::fireRate = 10;
uint16_t GameModeLogic::reloadTimer = 1000;
uint8_t GameModeLogic::prevTriggerButtonState = 1;
uint8_t GameModeLogic::prevReloadButtonState = 1;
uint8_t GameModeLogic::prevFlashlightButtonState = 1;
bool GameModeLogic::flashlightOn = false;

static const gpio_num_t allWeaponLeds[] = {
    PISTOL_LED, 
    MACHINE_GUN_LED, 
    RIFLE_LED, 
    ROCKET_LED
};

// Initialize all pins and components
void GameModeLogic::initialize()
{
    // Initialize all pins
    GPIOHelper::initializePinButton(TRIGGER_PIN);
    GPIOHelper::initializePinLED(FIREING_INDICATOR_LED);
    GPIOHelper::initializePinButton(PISTOL_BUTTON);
    GPIOHelper::initializePinLED(PISTOL_LED);
    GPIOHelper::initializePinButton(MACHINE_GUN_BUTTON);
    GPIOHelper::initializePinLED(MACHINE_GUN_LED);
    GPIOHelper::initializePinButton(RIFLE_BUTTON);
    GPIOHelper::initializePinLED(RIFLE_LED);
    GPIOHelper::initializePinButton(ROCKET_BUTTON);
    GPIOHelper::initializePinLED(ROCKET_LED);
    GPIOHelper::initializePinButton(RELOAD_BUTTON);
    GPIOHelper::initializePinLED(CLIP_LEVEL_RED);
    GPIOHelper::initializePinLED(CLIP_LEVEL_YELLO);
    GPIOHelper::initializePinLED(CLIP_LEVEL_GREEN);
    GPIOHelper::initializePinButton(FLASHLIGHT_BUTTON);
    GPIOHelper::initializePinLED(FLASHLIGHT_LED);
    GPIOHelper::initializePinLED(READY_TO_FIRE_1);
    GPIOHelper::initializePinLED(READY_TO_FIRE_2);
    GPIOHelper::initializePinLED(READY_TO_FIRE_3);
    GPIOHelper::initializePinLED(READY_TO_FIRE_4);
}

// Main game loop
void GameModeLogic::run()
{
    initialize();
    while (true)
    {
        updateClipLED(clipLevel, clipCapacity);
        int trigger_button_state = gpio_get_level(TRIGGER_PIN);
        int reload_button_state = gpio_get_level(RELOAD_BUTTON);
        int flashlight_button_state = gpio_get_level(FLASHLIGHT_BUTTON);

        // Fire if the button has been pressed
        if (trigger_button_state == 0 && prevTriggerButtonState == 1)
        {
            if (clipLevel > 0)
            {
                clipLevel -= 1;
                Blaster::fire(gunDamage);
                ESP_LOGI(TAG, "fired with ammo remaining: %d", clipLevel);
                indicateFireReady(fireRate);
            }
        }
        prevTriggerButtonState = trigger_button_state;

        if (flashlight_button_state == 0 && prevFlashlightButtonState == 1)
        {
            flashlightOn = !flashlightOn;
            gpio_set_level(FLASHLIGHT_LED, flashlightOn ? 1 : 0);
            ESP_LOGI(TAG, "Flashlight %s", flashlightOn ? "ON" : "OFF");
        }
        prevFlashlightButtonState = flashlight_button_state;

        if (!gpio_get_level(PISTOL_BUTTON))
        {
            GameModeLogic::selectGun(1, PISTOL_LED, 10, 10, 400);
        }
        if (!gpio_get_level(MACHINE_GUN_BUTTON))
        {
            GameModeLogic::selectGun(2, MACHINE_GUN_LED, 20, 10, 3000);
        }
        if (!gpio_get_level(RIFLE_BUTTON))
        {
            GameModeLogic::selectGun(3, RIFLE_LED, 4, 1000, 1000);
        }
        if (!gpio_get_level(ROCKET_BUTTON))
        {
            GameModeLogic::selectGun(4, ROCKET_LED, 1, 0, 7000);
        }
        if (reload_button_state == 0 && prevReloadButtonState == 1)
        {
            ESP_LOGI(TAG, "Reloading for: %d", reloadTimer);
            reload(reloadTimer);
            clipLevel = clipCapacity;
            ESP_LOGI(TAG, "Reloading complete: %d", reloadTimer);
        }
        prevReloadButtonState = reload_button_state;
        if(Blaster::health <= 0){
            ESP_LOGI(TAG, "Death Sequence Called");
            Blaster::deathSequence();
        }
        vTaskDelay(10);
    }
}

// Helper methods

void GameModeLogic::updateClipLED(uint8_t clipLevel, uint8_t clipCapacity)
{
    // Calculate the percentage of remaining ammo
    float ammoPercentage = (float)clipLevel / clipCapacity;

    // Turn off all LEDs first
    gpio_set_level(CLIP_LEVEL_GREEN, 0);
    gpio_set_level(CLIP_LEVEL_YELLO, 0);
    gpio_set_level(CLIP_LEVEL_RED, 0);

    if (ammoPercentage > 0.66)
    {
        // 66% or more, light up green
        gpio_set_level(CLIP_LEVEL_GREEN, 1);
        // ESP_LOGI(TAG, "Clip level: Green LED");
    }
    else if (ammoPercentage > 0.33)
    {
        // Between 33% and 66%, light up yellow
        gpio_set_level(CLIP_LEVEL_YELLO, 1);
        // ESP_LOGI(TAG, "Clip level: Yellow LED");
    }
    else if (ammoPercentage > 0)
    {
        // Less than 33%, light up red
        gpio_set_level(CLIP_LEVEL_RED, 1);
        // ESP_LOGI(TAG, "Clip level: Red LED");
    }
    else
    {
    }
}

void GameModeLogic::reload(uint16_t reloadTime)
{
    // Set up the intervals for the reload process
    uint16_t halfReloadTime = reloadTime / 2;
    uint16_t interval = reloadTime / 20; // Divide reload time into 20 smaller intervals for finer control

    // Start with red LED on and other LEDs off
    gpio_set_level(CLIP_LEVEL_RED, 1);
    gpio_set_level(CLIP_LEVEL_YELLO, 0);
    gpio_set_level(CLIP_LEVEL_GREEN, 0);
    ESP_LOGI(TAG, "Reload started. Red LED on.");

    for (uint16_t elapsed = 0; elapsed < reloadTime; elapsed += interval)
    {
        vTaskDelay(interval / portTICK_PERIOD_MS);

        // Update LEDs based on progress
        if (elapsed >= halfReloadTime)
        {
            // 50% complete: Turn yellow LED on and red LED off
            gpio_set_level(CLIP_LEVEL_RED, 0);
            gpio_set_level(CLIP_LEVEL_YELLO, 1);
            ESP_LOGI(TAG, "Reload half complete. Yellow LED on.");
        }

        if (elapsed >= reloadTime - interval)
        {
            // 100% complete: Turn green LED on and turn off others
            gpio_set_level(CLIP_LEVEL_YELLO, 0);
            gpio_set_level(CLIP_LEVEL_GREEN, 1);
            ESP_LOGI(TAG, "Reload complete. Green LED on.");
        }
    }
}

void GameModeLogic::indicateFireReady(uint16_t fireRate)
{
    // Divide fireRate into 4 intervals for each LED
    uint16_t interval = fireRate / 4;

    // Briefly indicate firing with the firing indicator LED
    gpio_set_level(FIREING_INDICATOR_LED, 1);
    vTaskDelay(50 / portTICK_PERIOD_MS); // Small delay to show the firing
    gpio_set_level(FIREING_INDICATOR_LED, 0);

    // Turn off all ready-to-fire LEDs initially
    gpio_set_level(READY_TO_FIRE_1, 0);
    gpio_set_level(READY_TO_FIRE_2, 0);
    gpio_set_level(READY_TO_FIRE_3, 0);
    gpio_set_level(READY_TO_FIRE_4, 0);

    // Light up each LED progressively
    gpio_set_level(READY_TO_FIRE_1, 1);
    vTaskDelay(interval / portTICK_PERIOD_MS);

    gpio_set_level(READY_TO_FIRE_2, 1);
    vTaskDelay(interval / portTICK_PERIOD_MS);

    gpio_set_level(READY_TO_FIRE_3, 1);
    vTaskDelay(interval / portTICK_PERIOD_MS);

    gpio_set_level(READY_TO_FIRE_4, 1);
    vTaskDelay(interval / portTICK_PERIOD_MS);
}

void GameModeLogic::selectGun(uint8_t damage, gpio_num_t ledPin, uint8_t capacity, uint16_t rate, uint16_t reloadTime)
{
    // Turn off all other LEDs
    for (int i = 0; i < sizeof(allWeaponLeds) / sizeof(allWeaponLeds[0]); i++) {
        gpio_set_level(allWeaponLeds[i], 0);
    }

    // Turn on the selected LED
    gpio_set_level(ledPin, 1);

    // Set gun attributes
    gunDamage = damage;
    clipCapacity = capacity;
    clipLevel = 0;
    fireRate = rate;
    reloadTimer = reloadTime;

    ESP_LOGI(TAG, "Gun selected: %d", gunDamage);
}