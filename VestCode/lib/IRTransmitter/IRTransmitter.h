#ifndef IR_TRANSMITTER_H
#define IR_TRANSMITTER_H

extern "C" {
    #include "ir_nec_encoder.h"
}

#include "driver/rmt_tx.h"

class IRTransmitter {
public:
    IRTransmitter() = delete;

    static void setup(gpio_num_t tx_pin, uint32_t resolution_hz);
    static void transmit(uint16_t address, uint16_t command);

private:
    static rmt_channel_handle_t tx_channel;
    static rmt_encoder_handle_t nec_encoder;
    static rmt_transmit_config_t transmit_config;

    static void setupRMT(gpio_num_t tx_pin, uint32_t resolution_hz);
};

#endif
