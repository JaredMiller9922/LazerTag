#ifndef VEST_H
#define VEST_H

#include <stdint.h>
#include "IRReceiver.h"
#include "IRTransmitter.h"
#include <string>
#include "RGB_LED.h"


class Vest {
public: 
    // Removes default constructor 
    Vest() = delete;

    // Instance variables 
    static int health;
    static uint8_t teamAddress;
    static uint8_t playerAddress;
    static IRTransmitter transmitter;
    static IRReceiver receiver;
    static RGB_LED rgbLed;

    static void setup();
    static uint8_t getTeamAddr();
    static uint8_t getPlayerAddr();
    static int takeDamage(int damage);
    static void fire();
    static void pairWithGun();
    static void gameSetUp();
    static void setTeamColor(uint8_t team);
    static bool getParingStatus();

private:
    static bool onCommandReceived(uint16_t address, uint16_t command);
    static void deathSequence();
    static void startReceiverTask(void* pvParameters);
};
#endif