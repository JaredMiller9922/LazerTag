#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LazerBlaster.h"
#include "esp_log.h"
#include "GPIOHelper.h"

#define TAG "Main"

#define TX_PIN GPIO_NUM_17 
#define RX_PIN GPIO_NUM_18
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




#define IR_RESOLUTION_HZ 1000000

#define RED_TEAM_ADDR 0x01
#define BLUE_TEAM_ADDR 0x02

#define PLAYER1_ADDR 0x01
#define PLAYER2_ADDR 0x02


void updateClipLED(uint8_t clipLevel, uint8_t clipCapacity) {
    // Calculate the percentage of remaining ammo
    float ammoPercentage = (float)clipLevel / clipCapacity;

    // Turn off all LEDs first
    gpio_set_level(CLIP_LEVEL_GREEN, 0);
    gpio_set_level(CLIP_LEVEL_YELLO, 0);
    gpio_set_level(CLIP_LEVEL_RED, 0);

    if (ammoPercentage > 0.66) {
        // 66% or more, light up green
        gpio_set_level(CLIP_LEVEL_GREEN, 1);
        //ESP_LOGI(TAG, "Clip level: Green LED");
    } else if (ammoPercentage > 0.33) {
        // Between 33% and 66%, light up yellow
        gpio_set_level(CLIP_LEVEL_YELLO, 1);
        //ESP_LOGI(TAG, "Clip level: Yellow LED");
    } else if (ammoPercentage >0 ) {
        // Less than 33%, light up red
        gpio_set_level(CLIP_LEVEL_RED, 1);
        //ESP_LOGI(TAG, "Clip level: Red LED");
    } else{
        
    }
}

void reload(uint16_t reloadTime) {
    // Set up the intervals for the reload process
    uint16_t halfReloadTime = reloadTime / 2;
    uint16_t interval = reloadTime / 20; // Divide reload time into 20 smaller intervals for finer control

    // Start with red LED on and other LEDs off
    gpio_set_level(CLIP_LEVEL_RED, 1);
    gpio_set_level(CLIP_LEVEL_YELLO, 0);
    gpio_set_level(CLIP_LEVEL_GREEN, 0);
    ESP_LOGI(TAG, "Reload started. Red LED on.");

    for (uint16_t elapsed = 0; elapsed < reloadTime; elapsed += interval) {
        vTaskDelay(interval / portTICK_PERIOD_MS);

        // Update LEDs based on progress
        if (elapsed >= halfReloadTime) {
            // 50% complete: Turn yellow LED on and red LED off
            gpio_set_level(CLIP_LEVEL_RED, 0);
            gpio_set_level(CLIP_LEVEL_YELLO, 1);
            ESP_LOGI(TAG, "Reload half complete. Yellow LED on.");
        }

        if (elapsed >= reloadTime - interval) {
            // 100% complete: Turn green LED on and turn off others
            gpio_set_level(CLIP_LEVEL_YELLO, 0);
            gpio_set_level(CLIP_LEVEL_GREEN, 1);
            ESP_LOGI(TAG, "Reload complete. Green LED on.");
        }
    }
}

void indicateFireReady(uint16_t fireRate) {
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

    // After the intervals, turn off all LEDs and set the firing indicator
    //gpio_set_level(READY_TO_FIRE_1, 0);
    //gpio_set_level(READY_TO_FIRE_2, 0);
    //gpio_set_level(READY_TO_FIRE_3, 0);
    //gpio_set_level(READY_TO_FIRE_4, 0);
}


extern "C" void app_main(void) {
    // Create a Lazer Blaster
    LazerBlaster player(BLUE_TEAM_ADDR, PLAYER1_ADDR, 5);
    // LazerBlaster player2(BLUE_TEAM_ADDR, PLAYER2_ADDR, 5);

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



    ESP_LOGI(TAG, "Player: %04X Team: %04X", player.getPlayerAddr(), player.getTeamAddr());
    // ESP_LOGI(TAG, "Player: %04X Team: %04X", player2.getPlayerAddr(), player2.getTeamAddr());

    // Begin paring process
    setupESPnow();
    while(paring){
        vTaskDelay(10);
        // if trigger pressed
        if (gpio_get_level(TRIGGER_PIN) == 0){
        // fire(ownMac)
            player.sendMacAddressIR();
        }
    }
    ESP_LOGI(TAG, "Gun: pared with vest");

    bool setUpComplete = false;
    setLife(0);
    while (!setUpComplete){
        ESP_LOGI(TAG, "getLife: %d", getLife());
        if (getLife() > 0){
            ESP_LOGI(TAG, "getTeam: %d", getTeam());
            setUpComplete = true;
        }
        vTaskDelay(100);
        player.setTeamColor(getTeam());
    }
    ESP_LOGI(TAG, "Setup Complete");
    ESP_LOGI(TAG, "Setup Complete");


    // Infinite Program Logic
    uint8_t selectedGun =  1;
    uint8_t clipCapacity = 10;
    uint8_t clipLevel = 10;
    uint16_t fireRate = 10;
    uint16_t reloadTimer = 1000;
    uint8_t prev_trigger_button_state = 1;
    uint8_t prev_reload_button_state = 1;
    uint8_t prev_flashlight_button_state = 1;
    bool flashlightOn = false;

    
    while(true) {
        updateClipLED(clipLevel, clipCapacity);
        int trigger_button_state = gpio_get_level(TRIGGER_PIN);
        int reload_button_state = gpio_get_level(RELOAD_BUTTON);
        int flashlight_button_state = gpio_get_level(FLASHLIGHT_BUTTON);

        
        // Fire if the button has been pressed
        if (trigger_button_state == 0 && prev_trigger_button_state == 1)
        {
            if(clipLevel > 0){
                clipLevel -= 1;
                player.fire(selectedGun);
                ESP_LOGI(TAG, "fired with ammo remaining: %d", clipLevel);
                indicateFireReady(fireRate);
            }
        }
        prev_trigger_button_state = trigger_button_state;

        if (flashlight_button_state == 0 && prev_flashlight_button_state == 1) {
            flashlightOn = !flashlightOn;
            gpio_set_level(FLASHLIGHT_LED, flashlightOn ? 1 : 0);
            ESP_LOGI(TAG, "Flashlight %s", flashlightOn ? "ON" : "OFF");
        }
        prev_flashlight_button_state = flashlight_button_state;

        if(!gpio_get_level(PISTOL_BUTTON)){
            gpio_set_level(PISTOL_LED, 1);
            gpio_set_level(MACHINE_GUN_LED,0);
            gpio_set_level(RIFLE_LED,0);
            gpio_set_level(ROCKET_LED, 0);
            selectedGun = 1;
            clipCapacity = 10;
            clipLevel = 0;
            fireRate = 10;
            reloadTimer = 400;
            ESP_LOGI(TAG, "Pistol button pressed. Selected Gun: %d", selectedGun);
        }
        if(!gpio_get_level(MACHINE_GUN_BUTTON)){
            gpio_set_level(PISTOL_LED, 0);
            gpio_set_level(MACHINE_GUN_LED,1);
            gpio_set_level(RIFLE_LED,0);
            gpio_set_level(ROCKET_LED, 0);
            selectedGun = 2;
            clipCapacity = 20;
            clipLevel = 0;
            fireRate = 10;
            reloadTimer = 3000;
           ESP_LOGI(TAG, "Machine Gun button pressed. Selected Gun: %d", selectedGun);
        }
        if(!gpio_get_level(RIFLE_BUTTON)){
            gpio_set_level(PISTOL_LED, 0);
            gpio_set_level(MACHINE_GUN_LED,0);
            gpio_set_level(RIFLE_LED,1);
            gpio_set_level(ROCKET_LED, 0);
            selectedGun = 3;
            clipCapacity = 4;
            clipLevel = 0;
            fireRate = 1000;
            reloadTimer = 1000;
            ESP_LOGI(TAG, "Rifle button pressed. Selected Gun: %d", selectedGun);
        }
        if(!gpio_get_level(ROCKET_BUTTON)){
            gpio_set_level(PISTOL_LED, 0);
            gpio_set_level(MACHINE_GUN_LED,0);
            gpio_set_level(RIFLE_LED,0);
            gpio_set_level(ROCKET_LED, 1);
            selectedGun = 20;
            clipCapacity = 1;
            clipLevel = 0;
            reloadTimer = 7000;
            ESP_LOGI(TAG, "Rocket button pressed. Selected Gun: %d", selectedGun);
        }
        if(reload_button_state == 0 && prev_reload_button_state == 1){
            ESP_LOGI(TAG, "Reloading for: %d", reloadTimer);
            reload(reloadTimer);
            clipLevel = clipCapacity;
            ESP_LOGI(TAG, "Reloading complete: %d", reloadTimer);
        }
        prev_reload_button_state = reload_button_state;
        vTaskDelay(10);
    }
    
    
    ESP_LOGI("gun main", "Program Terminated");
}

