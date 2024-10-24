#ifndef IR_RECEIVER_H
#define IR_RECEIVER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/rmt_rx.h"
#include <functional>

class IRReceiver {
public:
    IRReceiver(gpio_num_t rx_pin, uint32_t resolution_hz);
    IRReceiver(gpio_num_t rx_pin, uint32_t resolution_hz, std::function<bool(uint16_t, uint16_t)> callback);
    ~IRReceiver();
    
    void startReceiving();
private:
    static std::function<bool(uint16_t, uint16_t)> rx_done_blaster_cb;
    static uint16_t s_nec_code_address;
    static uint16_t s_nec_code_command;

    rmt_symbol_word_t raw_symbols[64];  // 64 symbols to hold the received data
    rmt_channel_handle_t rx_channel;
    QueueHandle_t receive_queue;
    rmt_receive_config_t receive_config;

    static bool rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data);
    void setupRMT(gpio_num_t rx_pin, uint32_t resolution_hz);
    bool nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols);
    bool nec_parse_frame_repeat(rmt_symbol_word_t *rmt_nec_symbols);
    bool nec_check_in_range(uint32_t signal_duration, uint32_t spec_duration);
    bool nec_parse_logic0(rmt_symbol_word_t *rmt_nec_symbols);
    bool nec_parse_logic1(rmt_symbol_word_t *rmt_nec_symbols);
    void processReceivedData(const rmt_rx_done_event_data_t* rx_data);


};

#endif
