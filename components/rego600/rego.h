/*!
 * Base definition for rego_interface with class definitions for both internal and external use.
 * 
 */

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <iostream>
#include <sstream>



namespace esphome {
namespace rego {


#define UART_MAX_READ 128
#define UART_RELEASE_DELAY 100
#define UART_LOCK_TIMEOUT 10000

#define SM_STATE_TIMEOUT 60000
#define STATE_IDLE 0
#define STATE_UPDATE_REQUEST 1
#define STATE_COMMAND_REQUEST 2
#define STATE_WAITING_FOR_RESPONSE 3
#define STATE_WAITING_FOR_ACC 4


class RegoInterfaceComponent : public Component {
public:
    // Function override declarations
    void dump_config() override;

    // Function declaration
	bool send_command(uint8_t cmd, uint16_t reg, uint16_t val);
    bool recieve_read_response(size_t *available, int16_t *result);
    bool receive_write_acc(size_t *available, int16_t *result);
	void flush_uart_rx();
    bool is_uart_locked();

    // Function definitions
    void set_model(std::string model){ this->model_ = model; }
    void set_uart_parent(esphome::uart::UARTComponent *parent) { this->uart_ = parent; }
    void lock_uart() { this->uart_lock_time_ = millis(); this->uart_lock_ = true; }
    void release_uart() { this->uart_release_lock_request_time_ = millis(); }
    std::string to_str() { return "Model: " + this->model_ + " UART: " + std::to_string(this->uart_->get_baud_rate()) + " baud"; }

protected:
    // Function definitions
    void int16_to_7bit_array(int16_t value, uint8_t *write_array); // convert int (16 bit) to array for sending
    std::string data_to_hexstr(const uint8_t *data, size_t len);

    // Thread mutex
	bool uart_lock_ = false;
	unsigned long uart_lock_time_ =  0;
	unsigned long uart_release_lock_request_time_ = 0;

    // Config parameters
    esphome::uart::UARTComponent *uart_{nullptr};
    std::string model_;
};



class RegoBase : public PollingComponent {
public:
    // Function override declarations
    void update() override { }
	void setup() override { }

    // Function declaration
	bool is_sm_state_timeout();
    void set_sm_state(uint8_t state);

    // Function definitions
    uint8_t get_sm_state() { return this->state_; } 
    void register_hub(RegoInterfaceComponent *hub){ this->hub_ = hub; }
    void set_rego_variable(std::uint16_t rego_variable) { this->rego_variable_ = rego_variable; }

protected:
    // Function declaration
    std::string int_to_hex(std::uint16_t value);

	// SM
	uint8_t state_ = STATE_IDLE;
	unsigned long entered_state_ = 0;

    // Config parameters
    RegoInterfaceComponent *hub_;
    std::uint16_t rego_variable_;
};


}  // namespace rego
}  // namespace esphome
