#include "rego_button.h"

namespace esphome {
namespace rego {



static const char *TAG = "rego.button";


void RegoButton::dump_config()
{
    ESP_LOGCONFIG(TAG, "Rego Button:");
    LOG_BUTTON("  ", "Button", this);
    ESP_LOGCONFIG(TAG, "  Rego variable: 0x%s", this->int_to_hex(this->rego_variable_).c_str());
    ESP_LOGCONFIG(TAG, "  Hub: %s", this->hub_->to_str().c_str());
}


void RegoButton::press_action()
{
	if (this->get_sm_state() == STATE_IDLE)
	{
		this->set_sm_state(STATE_COMMAND_REQUEST);
		ESP_LOGD(TAG, "\"%s\" entered STATE_COMMAND_REQUEST.", this->get_name().c_str());
	}
	else
	{
		ESP_LOGD(TAG, "\"%s\" not in STATE_IDLE, skipping change to STATE_COMMAND_REQUEST.", this->get_name().c_str());
	}
}


void RegoButton::loop()
{
	switch ( this->get_sm_state() )
	{

		case STATE_COMMAND_REQUEST:
		{
			if (this->hub_->is_uart_locked())
			{
				ESP_LOGVV(TAG, "\"%s\" UART locked in STATE_COMMAND_REQUEST, waiting.", this->get_name().c_str());
				break;
			}

			int16_t payload = 0x0001; // key press and wheel right payload
			
			if (this->rego_variable_ == 0xff00)  // Wheel left dummy register
			{
				this->rego_variable_ = 0x0044; // Actual wheel register
				payload = 0xFFFF; // Wheel left payload
			}
			else if (this->rego_variable_ == 0xff01) // Wheel right dummy reg
			{
				this->rego_variable_ = 0x0044; // Actual wheel register
			}

			if (this->hub_->send_command(0x01, this->rego_variable_, payload))
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
				this->set_sm_state(STATE_IDLE);
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
	}
}



}  // namespace rego
}  // namespace esphome
