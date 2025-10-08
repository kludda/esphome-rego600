#include "rego_number.h"

namespace esphome {
namespace rego {


static const char *TAG = "rego.number";


void RegoNumber::dump_config()
{
    ESP_LOGCONFIG(TAG, "Rego Number:");
    LOG_NUMBER("  ", "Number", this);
    ESP_LOGCONFIG(TAG, "  Rego variable: 0x%s", this->int_to_hex(this->rego_variable_).c_str());
    ESP_LOGCONFIG(TAG, "  Value factor: %f", this->value_factor_);
    ESP_LOGCONFIG(TAG, "  Hub: %s", this->hub_->to_str().c_str());
}


void RegoNumber::setup()
{
    ESP_LOGD(TAG, "\"%s\" restoring value.", this->get_name().c_str());
	this->update();
}


void RegoNumber::update()
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


void RegoNumber::control(float value)
{
	this->command_value_ = value;

	if (this->get_sm_state() == STATE_IDLE)
	{
		this->set_sm_state(STATE_COMMAND_REQUEST);
		ESP_LOGD(TAG, "\"%s\" entered STATE_COMMAND_REQUEST.", this->get_name().c_str());
	}
	else
	{
		// Skipping a command request does not work well for automations, so "schedule" a command state asap
		command_state_requested_ = true;
		ESP_LOGD(TAG, "\"%s\" not in STATE_IDLE, skipping change to STATE_COMMAND_REQUEST but requested change to STATE_COMMAND_REQUEST.", this->get_name().c_str());
	}
}


void RegoNumber::loop() {

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

		case STATE_COMMAND_REQUEST:
		{
			if (this->hub_->is_uart_locked())
			{
				ESP_LOGVV(TAG, "\"%s\" UART locked in STATE_COMMAND_REQUEST, waiting.", this->get_name().c_str());
				break;
			}
			

			if (this->hub_->send_command(0x03, this->rego_variable_, (int16_t)(this->command_value_ / this->value_factor_)))
			{
				this->hub_->lock_uart();
				this->set_sm_state(STATE_WAITING_FOR_ACC);
				ESP_LOGD(TAG, "\"%s\" entered STATE_WAITING_FOR_ACC.", this->get_name().c_str());
				break;
			}
			
			if(this->is_sm_state_timeout())
			{
				ESP_LOGW(TAG, "\"%s\" timeout in STATE_COMMAND_REQUEST.", this->get_name().c_str());
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

		case STATE_WAITING_FOR_ACC:
		{
			size_t available = 0;
			int16_t result = 0;

			if(this->hub_->receive_write_acc(&available, &result))
			{
				ESP_LOGD(TAG, "\"%s\" got positive response to write command (%u)", this->get_name().c_str() , result);
			}

			if (available > 0) // Got something (even if it wasn't good) so exit state
			{
				this->set_sm_state(STATE_UPDATE_REQUEST);
				ESP_LOGD(TAG, "\"%s\" entered STATE_UPDATE_REQUEST.", this->get_name().c_str());
				this->hub_->release_uart();
				break;
			}

			if(this->is_sm_state_timeout())
			{
				ESP_LOGW(TAG, "\"%s\" timeout in STATE_WAITING_FOR_ACC.", this->get_name().c_str());
				this->set_sm_state(STATE_IDLE);
				this->hub_->release_uart();
			}

			break;
		}
		
		default:
		{
			if(command_state_requested_)
			{
				this->control(this->command_value_);
				command_state_requested_ = false;
			}
		}
	}
}


}  // namespace rego
}  // namespace esphome
