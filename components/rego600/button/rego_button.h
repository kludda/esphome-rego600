#pragma once

#include "esphome.h"
#include "esphome/components/rego600/rego.h"


namespace esphome {
namespace rego {

class RegoButton: public button::Button, public RegoBase {
public:
	void dump_config() override;
	void setup() override { }
    void update() override { }
    void press_action() override;
    void loop() override;
};

}  // namespace rego
}  // namespace esphome
