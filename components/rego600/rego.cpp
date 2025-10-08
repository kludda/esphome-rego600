#include "rego.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/core/util.h"
#include "esphome/core/version.h"

namespace esphome {
namespace rego {

static const char *TAG = "rego";

//////// class RegoInterfaceComponent : public Component

void RegoInterfaceComponent::dump_config()
{
    ESP_LOGCONFIG(TAG, "Rego Interface:");
    //ESP_LOGCONFIG(TAG, "  UART device: %s", this->uart_);  // TODO: Need a "to_str" representation
    //ESP_LOGCONFIG(TAG, "  UART device: %s", std::to_string(this->uart_->get_baud_rate()));
    //ESP_LOGCONFIG(TAG, "  Model: %s", this->model_);  // TODO: Need a "to_str" representation
}


bool RegoInterfaceComponent::is_uart_locked()
{
	unsigned long now = millis();

	// check if min 100 ms since lock release. rego will not respond if new cmd is sent to fast.
	if(this->uart_release_lock_request_time_ > 0)
	{
		if ((unsigned long)(now - this->uart_release_lock_request_time_) > UART_RELEASE_DELAY)
		{
			this->uart_release_lock_request_time_ = 0;
			this->uart_lock_ = false;
			this->uart_lock_time_ = 0;
		} 
	}
	// Check if lock has been too long
	else if ((this->uart_lock_time_ > 0) && (unsigned long)(now - this->uart_lock_time_) > UART_LOCK_TIMEOUT)
	{
		ESP_LOGE(TAG, "UART lock timeout.");
		this->flush_uart_rx();
		this->uart_lock_ = false;
		this->uart_lock_time_ = 0;
	}

	return this->uart_lock_; 
}


void RegoInterfaceComponent::int16_to_7bit_array(int16_t value, uint8_t *write_array)
{
    *(write_array+2) = (uint8_t)value & 0x7F;
    *(write_array+1) = (uint8_t)(value>>7) & 0x7F;
    *write_array = (uint8_t)(value>>14) & 0x03;
}


std::string RegoInterfaceComponent::data_to_hexstr(const uint8_t *data, size_t len)
{
    std::stringstream ss;
    ss << std::hex;
    for( int i(0) ; i < len; ++i )
        ss << std::setw(2) << std::setfill('0') << (int)data[i];
    return ss.str();
}


void RegoInterfaceComponent::flush_uart_rx()
{
	uint8_t buf;
	while(this->uart_->available()) {
		this->uart_->read_byte(&buf);
	    ESP_LOGV(TAG, "Flushed UART RX: %u", buf);
	}
}


bool RegoInterfaceComponent::send_command(uint8_t cmd, uint16_t reg, uint16_t val)
{
    // Compose command
    uint8_t request[9];
    *request = 0x81;
    *(request+1) = cmd;
    this->int16_to_7bit_array(reg,request+2);
    this->int16_to_7bit_array(val,request+5);
    *(request+8) = 0;
	
	// calc checksum
    for (int i=2; i< 8; i++)
    {
        *(request+8) ^= *(request+i); // update XOR with data
    }

    // Send command
    ESP_LOGV(TAG, "Command to send: %s", this->data_to_hexstr(request, sizeof(request)).c_str());
	this->flush_uart_rx();
    this->uart_->write_array(request, sizeof(request));
    this->uart_->flush(); // wait until TX buffer is empty

    return true;
}


bool RegoInterfaceComponent::recieve_read_response(size_t *available, int16_t *result)
{
    // Read result
	uint8_t response[UART_MAX_READ];
	if ((*available = this->uart_->available()) > 0) {
		size_t read_bytes = std::min<size_t>(*available, UART_MAX_READ);
		this->uart_->read_array(response, read_bytes);
		ESP_LOGV(TAG, "Response received: %s", this->data_to_hexstr(response, *available).c_str());
	} else {
		return false;
	}

	if (*available != 5) {
		ESP_LOGW(TAG, "Response wrong size");
		return false;
	}

	if (! (response[0] == 0x01 || response[0] == 0x0C)) {
		ESP_LOGW(TAG, "Response from wrong address");
		return false;
	}

	if (response[4] != (response[1]^response[2]^response[3])) {
		ESP_LOGW(TAG, "Response wrong checksum");
		return false;
	}

	*result = ( (int16_t)*(response+1) << 14 ) | ( (int16_t)*(response+2) << 7 ) | (int16_t)*(response+3);
	ESP_LOGV(TAG, "Response decoded to %u", *result);

    return true;
}


bool RegoInterfaceComponent::receive_write_acc(size_t *available, int16_t *result)
{
    // Read result
	uint8_t response[UART_MAX_READ];
	if ((*available = this->uart_->available()) > 0) {
		size_t read_bytes = std::min<size_t>(*available, UART_MAX_READ);
		this->uart_->read_array(response, read_bytes);
		ESP_LOGV(TAG, "Response received: %s", this->data_to_hexstr(response, *available).c_str());
	} else {
		return false;
	}

    if (!(*available == 1 && response[0] == 0x01)) {
        *result = response[0];
        ESP_LOGW(TAG, "Negative response to write command (%u)", *result);
        return false;
    }

    return true;
}




//////// class RegoBase : public PollingComponent


void RegoBase::set_sm_state(uint8_t state)
{ 
	this->state_ = state; 
	this->entered_state_ = millis();
} 

bool RegoBase::is_sm_state_timeout()
{
	unsigned long now = millis();
	if ((unsigned long)(now - this->entered_state_) > SM_STATE_TIMEOUT)
	{
		return true;
	}
	return false;
}

std::string RegoBase::int_to_hex(std::uint16_t value)
{
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(sizeof(value)*2) << std::hex << value;
	return stream.str();
}






}  // namespace rego
}  // namespace esphome
