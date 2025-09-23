#pragma once

#include "esphome.h"
#include "esphome/components/rego600/rego.h"

namespace esphome {
namespace rego {

class RegoNumber : public number::Number, public RegoBase {
public:
    void dump_config() override;
    void setup() override;
    void update() override;
    void control(float value) override;
    void loop() override;
    void set_value_factor(float value_factor) { this->value_factor_ = value_factor; }
protected:
    float value_factor_;
    float command_value_;
    bool command_state_requested_ = false;	

};

}  // namespace rego
}  // namespace esphome
