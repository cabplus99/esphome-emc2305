#include "emc2305_sensor.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace emc2305 {

static const char *const TAG = "EMC2305.sensor";

float EMC2305Sensor::get_setup_priority() const { return setup_priority::DATA; }

void EMC2305Sensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Emc2305 sensor:");
  ESP_LOGCONFIG(TAG, "  Channel: %u", this->channel_);
  LOG_SENSOR("  ", "Speed", this->speed_sensor_);
  LOG_SENSOR("  ", "Duty cycle", this->duty_cycle_sensor_);

}

void EMC2305Sensor::update() {
  if (this->speed_sensor_ != nullptr) {
    float speed = this->parent_->get_speed(this->channel_);
    this->speed_sensor_->publish_state(speed);
  }

  if (this->duty_cycle_sensor_ != nullptr) {
    float duty_cycle = this->parent_->get_duty_cycle(this->channel_);
    this->duty_cycle_sensor_->publish_state(duty_cycle * 100.0f);
  }
}

}  // namespace emc2305
}  // namespace esphome
