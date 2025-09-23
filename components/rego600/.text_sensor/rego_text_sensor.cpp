#include "rego_text_sensor.h"

namespace esphome {
namespace rego {


/*
This component is not adapted to the new state machine arch.
Encoding the response to UTF-8 does not work properly.
Code is left for reference for future development.
*/



/*
heat-pump.yaml


text_sensor:
  - platform: rego600
    name: Line 1
    rego_variable: 0x0000

  - platform: rego600
    name: Line 2
    rego_variable: 0x0001

  - platform: rego600
    name: Line 3
    rego_variable: 0x0002
  
  - platform: rego600
    name: Line 4
    rego_variable: 0x0003


*/


/*
rego.h

class RegoInterfaceComponent : public Component {
public:
    bool read_text(int16_t reg, std::string *result);

protected:
	//std::string latin9_to_utf8(const std::string input);
	//std::string latin9_to_utf8(const uint8_t *input);  // https://github.com/dankar/rego600/tree/master

};
*/


/*
rego.cpp

bool RegoInterfaceComponent::read_text(int16_t reg, std::string *result)
{
    // ESP_LOGD(TAG, "Processing read of register %s (%u)", this->data_to_hexstr, this->data_to_hexstr(reg, sizeof(reg)).c_str(), reg);
    size_t available = 0;
    uint8_t response[MAX_READ];
    if (! this->command_and_response(0x81, 0x20, reg, 0x00, &available, response))
    {
        return false;
    }

    if (available < 42) {
        ESP_LOGE(TAG, "Response too short %u", available);
        return false;
    }

    if (! (response[0] == 0x01 || response[0] == 0x0C)) {
        ESP_LOGE(TAG, "Response from wrong address");
        return false;
    }

    uint8_t calc_checksum = 0;
    for (int i=1; (i+1)<available; i++)
    {
        calc_checksum ^= response[i];
    }
	
    if (response[available - 1] != calc_checksum) {
        ESP_LOGE(TAG, "Response wrong checksum");
        return false;
    }


    // The following is supposed to create a string that the esphome webserver and home assistant can display, but it fails to do so.

	std::string res;
	for (int i=1; (i+1)<available; i+=2) {
		res.push_back(((response[i] & 0xf) << 4) | (response[i + 1] & 0xf));
	}
	
	ESP_LOGD(TAG, "Response decoded 8bit to %s", res);
	
	result->clear();
	result->append(latin9_to_utf8(res));
	
	ESP_LOGD(TAG, "Response utf8 %s", result);
	 
    return true;
}

std::string RegoInterfaceComponent::latin9_to_utf8(const std::string input) // https://github.com/dankar/rego600/tree/master
{
    std::string result;
    size_t      n = 0;

    for (size_t i = 1; i < 42; i++) {
        if (input[i] < 128) {
            result.push_back(input[i]);
        } else if (input[i] < 192)
            switch (input[i]) {
            case 164:
                result.push_back(226);
                result.push_back(130);
                result.push_back(172);
                break;
            case 166:
                result.push_back(197);
                result.push_back(160);
                break;
            case 168:
                result.push_back(197);
                result.push_back(161);
                break;
            case 180:
                result.push_back(197);
                result.push_back(189);
                break;
            case 184:
                result.push_back(197);
                result.push_back(190);
                break;
            case 188:
                result.push_back(197);
                result.push_back(146);
                break;
            case 189:
                result.push_back(197);
                result.push_back(147);
                break;
            case 190:
                result.push_back(197);
                result.push_back(184);
                break;
            default:
                result.push_back(194);
                result.push_back(input[i]);
                break;
            }
        else {
            result.push_back(195);
            result.push_back(input[i] - 64);
        }
    }

    return result;
}
*/

















static const char *TAG = "rego.text_sensor";

void RegoTextSensor::dump_config() {
    ESP_LOGCONFIG(TAG, "Rego TextSensor:");
    LOG_TEXT_SENSOR("  ", "TextSensor", this);
    ESP_LOGCONFIG(TAG, "  Rego variable: 0x%s", this->int_to_hex(this->rego_variable_).c_str());
    ESP_LOGCONFIG(TAG, "  Hub: %s", this->hub_->to_str().c_str());
}

void RegoTextSensor::update() {
    std::string result = "";
    if (this->hub_->read_text(this->rego_variable_, &result)) {
        this->publish_state(result);
    }
    else {
        ESP_LOGE(TAG, "Could not update text_sensor \"%s\"", this->get_name().c_str());
    }
}

}  // namespace rego
}  // namespace esphome
