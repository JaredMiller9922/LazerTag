#ifndef IR_RECEIVER_H
#define IR_RECEIVER_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/rmt_rx.h"
#include "driver/gpio.h"
#include <functional>

class IRReceiver {
public:
    static void setup(gpio_num_t rx_pin, uint32_t resolution_hz, std::function<bool(uint16_t, uint16_t)> callbackArg = nullptr);
    static void startReceiving();
    static rmt_symbol_word_t raw_symbols[64];  // 64 symbols to hold the received data
    static rmt_channel_handle_t rx_channel;
    static QueueHandle_t receive_queue;
    static rmt_receive_config_t receive_config;

private:
    static std::function<bool(uint16_t, uint16_t)> rx_done_blaster_cb;
    static uint16_t s_nec_code_address;
    static uint16_t s_nec_code_command;

    static bool rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data);
    static void setupRMT(gpio_num_t rx_pin, uint32_t resolution_hz);
    static bool nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols);
    static bool nec_parse_frame_repeat(rmt_symbol_word_t *rmt_nec_symbols);
    static bool nec_check_in_range(uint32_t signal_duration, uint32_t spec_duration);
    static bool nec_parse_logic0(rmt_symbol_word_t *rmt_nec_symbols);
    static bool nec_parse_logic1(rmt_symbol_word_t *rmt_nec_symbols);
    static void processReceivedData(const rmt_rx_done_event_data_t* rx_data);
};

#endif
