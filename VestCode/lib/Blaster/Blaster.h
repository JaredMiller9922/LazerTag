#ifndef BLASTER_H
#define BLASTER_H

#include <stdint.h>
#include "IRReceiver.h"
#include "IRTransmitter.h"
#include <string>
#include "RGB_LED.h"


class Blaster {
public: 
    // Removes default constructor 
    Blaster() = delete;

    // Instance variables 
    static int health;
    static uint8_t teamAddress;
    static uint8_t playerAddress;
    static RGB_LED rgbLed;

    static void setup();
    static void sendMacAddressIR();
    static uint8_t getTeamAddr();
    static uint8_t getPlayerAddr();
    static int takeDamage(int damage);
    static void fire(uint8_t gunType);
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
