#ifndef LAZER_BLASTER_H
#define LAZER_BLASTER_H

#include <stdint.h>
#include "IRReceiver.h"
#include "IRTransmitter.h"
#include <string>
#include "RGB_LED.h"


class LazerBlaster {
public: 
    LazerBlaster(uint8_t teamAddr, uint8_t playerAddr, int startingHealth);
    uint8_t getTeamAddr();
    uint8_t getPlayerAddr();
    int takeDamage(int damage);
    void fire();
    void pairWithGun();
    void gameSetUp();
    void setTeamColor(uint8_t team);
    bool getParingStatus();

private:
    bool onCommandReceived(uint16_t address, uint16_t command);
    void deathSequence();
    static void startReceiverTask(void* pvParameters);
    uint8_t teamAddress;
    uint8_t playerAddress;
    int health;
    IRTransmitter transmitter;
    IRReceiver receiver;
    RGB_LED rgbLed;
};
#endif
