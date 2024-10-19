#ifndef LAZER_BLASTER_H
#define LAZER_BLASTER_H

#include <stdint.h>
#include "IRReceiver.h"
#include "IRTransmitter.h"

class LazerBlaster {
public: 
    LazerBlaster(uint8_t teamAddr, uint8_t playerAddr, int startingHealth);
    uint8_t getTeamAddr();

    uint8_t getPlayerAddr();
    int takeDamage(int damage);

    void fire();
private:
    void onCommandReceived(uint16_t command, uint16_t address);
    static void startReceiverTask(void* pvParameters);
    void pariWithVest();
    uint8_t teamAddress;
    uint8_t playerAddress;
    int health;
    IRTransmitter transmitter;
    IRReceiver receiver;
};
#endif
