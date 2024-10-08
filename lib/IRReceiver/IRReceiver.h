#ifndef IR_RECEIVER_H
#define IR_RECEIVER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/rmt_rx.h"

class IRReceiver {
public:
    IRReceiver(gpio_num_t rx_pin, uint32_t resolution_hz);
    ~IRReceiver();
    
    void startReceiving();
private:
    rmt_channel_handle_t rx_channel;
    QueueHandle_t receive_queue;
    rmt_receive_config_t receive_config;

    void setupRMT(gpio_num_t rx_pin, uint32_t resolution_hz);
    static bool rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data);
    static bool nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols);
    static bool nec_parse_frame_repeat(rmt_symbol_word_t *rmt_nec_symbols);
    static bool nec_check_in_range(uint32_t signal_duration, uint32_t spec_duration);
    static bool IRReceiver::nec_parse_logic0(rmt_symbol_word_t *rmt_nec_symbols) {};
    static bool IRReceiver::nec_parse_logic1(rmt_symbol_word_t *rmt_nec_symbols) {};

};

#endif
