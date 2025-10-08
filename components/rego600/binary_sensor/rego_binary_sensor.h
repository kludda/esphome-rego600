#pragma once

#include "esphome.h"
#include "esphome/components/rego600/rego.h"

namespace esphome {
namespace rego {

class RegoBinarySensor : public binary_sensor::BinarySensor, public RegoBase {
public:
    void dump_config() override;
	void setup() override;
    void update() override;
	void loop() override;
};

}  // namespace rego
}  // namespace esphome
