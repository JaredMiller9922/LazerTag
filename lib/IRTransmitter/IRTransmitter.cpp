#include "IRTransmitter.h"
#include "esp_log.h"

#define TAG "IRTransmitter"

/**
 * @brief Constructor for creating an IRTransmitter
 */
IRTransmitter::IRTransmitter(gpio_num_t tx_pin, uint32_t resolution_hz) {
    setupRMT(tx_pin, resolution_hz);
}

/**
 * @brief Constructor for creating an IRTransmitter
 */
IRTransmitter::~IRTransmitter() {
    rmt_disable(tx_channel);
    rmt_del_channel(tx_channel);
}

/**
 * @brief First creates a TX channel on the RMT. It then sets the carrier of that channel. Next it
 * configures that channel to use the encoder created for the NEC protocol. We then set the loop_count
 * which tells the transmission how many times it should repeat. Once all of this is configured we
 * enable the RMT channel we created.
 */
void IRTransmitter::setupRMT(gpio_num_t tx_pin, uint32_t resolution_hz) {
    // Configure the RMT TX channel
    rmt_tx_channel_config_t tx_channel_cfg = {
        .gpio_num = tx_pin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = resolution_hz,
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_channel_cfg, &tx_channel));

    // Apply the carrier for the TX channel
    rmt_carrier_config_t carrier_cfg = {
        .frequency_hz = 38000, // 38kHz carrier frequency
        .duty_cycle = 0.33,
    };
    ESP_ERROR_CHECK(rmt_apply_carrier(tx_channel, &carrier_cfg));

    // Install the NEC encoder
    ir_nec_encoder_config_t nec_encoder_cfg = {
        .resolution = resolution_hz,
    };
    ESP_ERROR_CHECK(rmt_new_ir_nec_encoder(&nec_encoder_cfg, &nec_encoder));

    // Set the transmit configuration
    transmit_config = {
        .loop_count = 0, // No loop
    };

    ESP_ERROR_CHECK(rmt_enable(tx_channel));
}

/**
 * @brief Transmits the address and command that we specify using the NEC encoder that was configured 
 * in setupRMT.
 */
void IRTransmitter::transmit(uint16_t address, uint16_t command) {
    ir_nec_scan_code_t scan_code = {
        .address = address,
        .command = command,
    };

    ESP_LOGI(TAG, "Transmitting NEC packet: Address=0x%04X, Command=0x%04X", address, command);
    ESP_ERROR_CHECK(rmt_transmit(tx_channel, nec_encoder, &scan_code, sizeof(scan_code), &transmit_config));
}
