#pragma once

#include "../emc2305.h"
#include "esphome/components/output/float_output.h"
#include "esphome/core/component.h"

namespace esphome {
namespace emc2305 {

/// This class allows to control the EMC2305 output.
class EMC2305Output : public esphome::Component, public esphome::output::FloatOutput {
 public:
  void set_channel(uint8_t channel) { channel_ = channel; }
  uint8_t get_channel() const { return channel_; }
  EMC2305Output(Emc2305Component *parent) : parent_(parent) {}

 protected:
  uint8_t channel_;
  void write_state(float state) override;

  Emc2305Component *parent_;
};

}  // namespace emc2305
}  // namespace esphome
