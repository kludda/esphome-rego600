#include "rego_sensor.h"

namespace esphome {
namespace rego {

static const char *TAG = "rego.sensor";


void RegoSensor::dump_config()
{
    ESP_LOGCONFIG(TAG, "Rego Sensor:");
    LOG_SENSOR("  ", "Sensor", this);
    ESP_LOGCONFIG(TAG, "  Rego variable: 0x%s", this->int_to_hex(this->rego_variable_).c_str());
    ESP_LOGCONFIG(TAG, "  Value factor: %f", this->value_factor_);
    ESP_LOGCONFIG(TAG, "  Hub: %s", this->hub_->to_str().c_str());
}


void RegoSensor::setup()
{
    ESP_LOGD(TAG, "\"%s\" restoring value.", this->get_name().c_str());
	this->update();
}


void RegoSensor::update()
{
	if (this->get_sm_state() == STATE_IDLE)
	{
		this->set_sm_state(STATE_UPDATE_REQUEST);
		ESP_LOGD(TAG, "\"%s\" entered STATE_UPDATE_REQUEST.", this->get_name().c_str());
	}
	else
	{
		ESP_LOGD(TAG, "\"%s\" not in STATE_IDLE, skipping change to STATE_UPDATE_REQUEST.", this->get_name().c_str());
	}
}


void RegoSensor::loop()
{

	switch ( this->get_sm_state() )
	{

		case STATE_UPDATE_REQUEST:
		{
			if (this->hub_->is_uart_locked())
			{
				ESP_LOGVV(TAG, "\"%s\" UART locked in STATE_UPDATE_REQUEST, waiting.", this->get_name().c_str());
				break;
			}

			if (this->hub_->send_command(0x02, this->rego_variable_, 0x00))
			{
				this->hub_->lock_uart();
				this->set_sm_state(STATE_WAITING_FOR_RESPONSE);
				ESP_LOGD(TAG, "\"%s\" entered STATE_WAITING_FOR_RESPONSE.", this->get_name().c_str());
				break;
			}
			
			if(this->is_sm_state_timeout()) {
				ESP_LOGW(TAG, "\"%s\" timeout in STATE_UPDATE_REQUEST.", this->get_name().c_str());
				this->set_sm_state(STATE_IDLE);
			}
			
			break;
		}

		case STATE_WAITING_FOR_RESPONSE:
		{
			size_t available = 0;
			int16_t result = 0;

			if(this->hub_->recieve_read_response(&available, &result))
			{
				this->publish_state(result * this->value_factor_);
			}

			if(available > 0) // Got something (even if it wasn't good) so exit state
			{ 
				this->set_sm_state(STATE_IDLE);
				ESP_LOGD(TAG, "\"%s\" entered STATE_IDLE.", this->get_name().c_str());
				this->hub_->release_uart();
				break;
			}
			
			if(this->is_sm_state_timeout())
			{
				ESP_LOGW(TAG, "\"%s\" timeout in STATE_WAITING_FOR_RESPONSE.", this->get_name().c_str());
				this->set_sm_state(STATE_IDLE);
				this->hub_->release_uart();
			}

			break;
		}

	}
}



}  // namespace rego
}  // namespace esphome
