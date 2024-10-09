#include "IRReceiver.h"
#include "esp_log.h"

#define IR_NEC_DECODE_MARGIN 200
#define TAG "IRReceiver"

#define NEC_LEADING_CODE_DURATION_0  9000
#define NEC_LEADING_CODE_DURATION_1  4500
#define NEC_PAYLOAD_ZERO_DURATION_0  560
#define NEC_PAYLOAD_ZERO_DURATION_1  560
#define NEC_PAYLOAD_ONE_DURATION_0   560
#define NEC_PAYLOAD_ONE_DURATION_1   1690
#define NEC_REPEAT_CODE_DURATION_0   9000
#define NEC_REPEAT_CODE_DURATION_1   2250

#define RED_TEAM_COMMAND 0x01
#define BLUE_TEAM_COMMAND 0x02

static uint16_t s_nec_code_address;
static uint16_t s_nec_code_command;

/**
 * @brief Constructor for the IRReceiver
 */
IRReceiver::IRReceiver(gpio_num_t rx_pin, uint32_t resolution_hz) {
    setupRMT(rx_pin, resolution_hz);
}

/**
 * @brief Destructor for the IRReceiver
 */
IRReceiver::~IRReceiver() {
    rmt_disable(rx_channel);
    rmt_del_channel(rx_channel);
}

/**
 * @brief Creates an RMT channel for receiving. Creates the receive queue to be used by the 
 * rx_done_callback when the RX signal is done being received.
 */
void IRReceiver::setupRMT(gpio_num_t rx_pin, uint32_t resolution_hz) {
    rmt_rx_channel_config_t rx_channel_cfg = {
        .gpio_num = rx_pin,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = resolution_hz,
        .mem_block_symbols = 64,
    };
    ESP_ERROR_CHECK(rmt_new_rx_channel(&rx_channel_cfg, &rx_channel));

    receive_queue = xQueueCreate(1, sizeof(rmt_rx_done_event_data_t));
    assert(receive_queue);

    rmt_rx_event_callbacks_t cbs = {
        .on_recv_done = rx_done_callback,
    };
    ESP_ERROR_CHECK(rmt_rx_register_event_callbacks(rx_channel, &cbs, receive_queue));

    receive_config = {
        .signal_range_min_ns = 1250,
        .signal_range_max_ns = 12000000, 
    };

    ESP_ERROR_CHECK(rmt_enable(rx_channel));
}

/**
 * @brief Callback that handles the end of the data being received. Creates a recieve queue that is used 
 * to manage data received.
 */
bool IRReceiver::rx_done_callback(rmt_channel_handle_t channel, const rmt_rx_done_event_data_t *edata, void *user_data) {
    BaseType_t high_task_wakeup = pdFALSE;
    QueueHandle_t receive_queue = (QueueHandle_t)user_data;
    xQueueSendFromISR(receive_queue, edata, &high_task_wakeup);
    return high_task_wakeup == pdTRUE;
}

/**
 * @brief Use the receive_queue to process the data that we have received. This method calls nec_parse_frame
 * which parses the address and the command from the next item in the receive_queue.
 */
void IRReceiver::startReceiving() {
    ESP_LOGI(TAG, "Entered the startReceiving method");

    rmt_rx_done_event_data_t rx_data;

    // Start receiving RMT symbols
    ESP_ERROR_CHECK(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols), &receive_config));

    // Wait for received data
    if (xQueueReceive(receive_queue, &rx_data, pdMS_TO_TICKS(1000)) == pdPASS) {
        size_t num_symbols = rx_data.num_symbols;

        // Log the number of received symbols
        ESP_LOGI(TAG, "Number of symbols received: %d", num_symbols);

        // Print the first symbol if available
        if (num_symbols > 0) {
            ESP_LOGI(TAG, "First Symbol: Level0=%d, Duration0=%d, Level1=%d, Duration1=%d", 
                     rx_data.received_symbols[0].level0, rx_data.received_symbols[0].duration0,
                     rx_data.received_symbols[0].level1, rx_data.received_symbols[0].duration1);
        } else {
            ESP_LOGI(TAG, "No symbols received.");
        }

        // Print all symbols for inspection
        for (size_t i = 0; i < num_symbols; i++) {
            ESP_LOGI(TAG, "Symbol[%d]: Level0=%d, Duration0=%d, Level1=%d, Duration1=%d", 
            i, rx_data.received_symbols[i].level0, rx_data.received_symbols[i].duration0,
            rx_data.received_symbols[i].level1, rx_data.received_symbols[i].duration1);
        }

        // Check if it's a normal frame or a repeat frame
        if (num_symbols == 34) {  // 34 symbols indicate a normal NEC frame
            if (nec_parse_frame(rx_data.received_symbols)) {
                ESP_LOGI(TAG, "Received Address=%04X, Command=%04X", s_nec_code_address, s_nec_code_command);
            }
        } 
        else if (num_symbols == 2) {  // 2 symbols indicate a repeat frame
            ESP_LOGI(TAG, "Ignoring Repeat Frames");
        } 
        else {
            ESP_LOGI(TAG, "Incorrect number of symbols received! num_symbols = %d", num_symbols);
        }

        // Start receiving again
        ESP_ERROR_CHECK(rmt_receive(rx_channel, raw_symbols, sizeof(raw_symbols), &receive_config));
    }
}




/**
 * @brief Ensure the signal duration is within the NEC protocol with a small error margin
 */
bool IRReceiver::nec_check_in_range(uint32_t signal_duration, uint32_t spec_duration) {
    return (signal_duration < (spec_duration + IR_NEC_DECODE_MARGIN)) &&
           (signal_duration > (spec_duration - IR_NEC_DECODE_MARGIN));
}

/**
 * @brief Parse the actual frame received and store the address and the command
 */
bool IRReceiver::nec_parse_frame(rmt_symbol_word_t *rmt_nec_symbols) {
    rmt_symbol_word_t *cur = rmt_nec_symbols;
    uint16_t address = 0;
    uint16_t command = 0;

    bool valid_leading_code = nec_check_in_range(cur->duration0, NEC_LEADING_CODE_DURATION_0) &&
                              nec_check_in_range(cur->duration1, NEC_LEADING_CODE_DURATION_1);

    if (!valid_leading_code) return false;
    cur++;

   for (int i = 0; i < 16; i++) {
        if (nec_parse_logic1(cur)) {
            address |= 1 << i;
        } else if (nec_parse_logic0(cur)) {
            address &= ~(1 << i);
        } else {
            return false;
        }
        cur++;
    }

    for (int i = 0; i < 16; i++) {
        if (nec_parse_logic1(cur)) {
            command |= 1 << i;
        } else if (nec_parse_logic0(cur)) {
            command &= ~(1 << i);
        } else {
            return false;
        }
        cur++;
    }

    s_nec_code_address = address;
    s_nec_code_command = command;
    ESP_LOGI(TAG, "Size of address at the end of nec_parse_frame: %d bytes", sizeof(s_nec_code_address));
    ESP_LOGI(TAG, "Size of command at the end of nec_parse_frame: %d bytes", sizeof(s_nec_code_command));
    return addressCommandHandler(address, command);
}

bool IRReceiver::addressCommandHandler(uint16_t address, uint16_t command){
    // This came from a red teams gun
    if (command == RED_TEAM_COMMAND){
        ESP_LOGI(TAG, "Red Team IR Sent");
        
    }
    else if (command == BLUE_TEAM_COMMAND){
        ESP_LOGI(TAG, "Blue Team IR Sent");
    }
    else {
        ESP_LOGI(TAG, "Unrecognized command");
    }

    // Return true because they're weren't any issues
    return true;
}

/**
 * @brief Ensure that the logical 0 is within NECs timing range.
 */
bool IRReceiver::nec_parse_logic0(rmt_symbol_word_t *rmt_nec_symbols) {
    return nec_check_in_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ZERO_DURATION_0) &&
           nec_check_in_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ZERO_DURATION_1);
}

/**
 * @brief Ensure that the logical 1 is within NECs timing range.
 */
bool IRReceiver::nec_parse_logic1(rmt_symbol_word_t *rmt_nec_symbols) {
    return nec_check_in_range(rmt_nec_symbols->duration0, NEC_PAYLOAD_ONE_DURATION_0) &&
           nec_check_in_range(rmt_nec_symbols->duration1, NEC_PAYLOAD_ONE_DURATION_1);
}