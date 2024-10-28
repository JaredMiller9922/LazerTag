#ifndef LAZER_BLASTER_H
#define LAZER_BLASTER_H

#include <stdint.h>
#include "IRReceiver.h"
#include "IRTransmitter.h"
#include "ESPnow.h"
#include <string>

class LazerBlaster {
public: 
    LazerBlaster(uint8_t teamAddr, uint8_t playerAddr, int startingHealth);
    uint8_t getTeamAddr();

    uint8_t getPlayerAddr();
    int takeDamage(int damage);

    void fire();
    void pairWithVest();
    void sendMacAddressIR();
    std::string getTeamColor(uint8_t);
    void setTeam(uint8_t newteam);
    void setLife(uint8_t newLife);


private:
    bool onCommandReceived(uint16_t address, uint16_t command);
    void deathSequence();
    static void startReceiverTask(void* pvParameters);
    uint8_t teamAddress;
    uint8_t playerAddress;
    int health;
    IRTransmitter transmitter;
    IRReceiver receiver;
};
#endif
