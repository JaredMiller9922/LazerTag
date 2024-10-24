#ifndef IR_TRANSMITTER_H
#define IR_TRANSMITTER_H

extern "C" {
    #include "ir_nec_encoder.h"
}

#include "driver/rmt_tx.h"

class IRTransmitter {
public:
    IRTransmitter(gpio_num_t tx_pin, uint32_t resolution_hz);
    ~IRTransmitter();

    void transmit(uint16_t address, uint16_t command);

private:
    rmt_channel_handle_t tx_channel;
    rmt_encoder_handle_t nec_encoder;
    rmt_transmit_config_t transmit_config;

    void setupRMT(gpio_num_t tx_pin, uint32_t resolution_hz);
};

#endif
