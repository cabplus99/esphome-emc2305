#include "emc2305_output.h"
#include "esphome/core/log.h"

namespace esphome {
namespace emc2305 {

static const char *const TAG = "emc2305.output";

void Emc2305Output::write_state(float state) {
  if (!this->parent_) {
    ESP_LOGE(TAG, "Parent not set for output channel %d", this->channel_);
    return;
  }
  this->parent_->set_duty_cycle(this->channel_, state);
  ESP_LOGD(TAG, "[0x%02X] Fan %d output set to %.1f%%", this->parent_->get_i2c_address(), this->channel_ + 1, state * 100.0f);
}

}  // namespace emc2305
}  // namespace esphome
