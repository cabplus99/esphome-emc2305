#pragma once

#include "esphome/components/output/float_output.h"
#include "../emc2305.h"

namespace esphome {
namespace emc2305 {

class Emc2305Output : public output::FloatOutput {
 public:
  void set_parent(Emc2305Component *parent) { this->parent_ = parent; }
  void set_channel(uint8_t channel) { this->channel_ = channel; }

  void write_state(float state) override;

 protected:
  Emc2305Component *parent_{nullptr};
  uint8_t channel_{0};
};

}  // namespace emc2305
}  // namespace esphome
