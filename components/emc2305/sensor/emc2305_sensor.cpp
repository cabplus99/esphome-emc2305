#include "emc2305_sensor.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace emc2305 {

static const char *const TAG = "emc2305.sensor";

float Emc2305Sensor::get_setup_priority() const { return setup_priority::DATA; }

void Emc2305Sensor::dump_config() {
  ESP_LOGCONFIG(TAG, "EMC2305 Sensor (fan %d on 0x%02X):", this->channel_ + 1, this->parent_->get_i2c_address());
  LOG_SENSOR("  ", "Speed (RPM)", this->speed_sensor_);
  LOG_SENSOR("  ", "Duty cycle (%)", this->duty_cycle_sensor_);
  LOG_BINARY_SENSOR("  ", "Stall", this->stall_sensor_);
  LOG_BINARY_SENSOR("  ", "Drive Fail", this->df_sensor_);
}

void Emc2305Sensor::update() {
  if (!this->parent_) {
    ESP_LOGE(TAG, "Parent EMC2305 component is null! Skipping update.");
    return;
  }

  // --- Duty cycle ---
  float duty_cycle = this->parent_->get_duty_cycle(this->channel_);
  if (this->duty_cycle_sensor_ != nullptr) {
    this->duty_cycle_sensor_->publish_state(duty_cycle * 100.0f);
  }

  // --- Speed (RPM) ---
  float speed = 0.0f;
  if (duty_cycle > 0.0f) {
    speed = this->parent_->get_speed(this->channel_);
  }
  if (this->speed_sensor_ != nullptr) {
    this->speed_sensor_->publish_state(speed);
  }

  // --- Stall status (0x25) ---
  uint8_t stall_reg = 0;
  auto err1 = this->parent_->read_register(0x25, &stall_reg, 1, true);

  if (err1 != esphome::i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to read stall status (0x25) from EMC2305 at 0x%02X",
             this->parent_->get_i2c_address());
    return;
  }

  // --- Drive fail status (0x27) ---
  uint8_t df_reg = 0;
  auto err2 = this->parent_->read_register(0x27, &df_reg, 1, true);

  if (err2 != esphome::i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Failed to read drive fail status (0x27) from EMC2305 at 0x%02X",
             this->parent_->get_i2c_address());
    return;
  }

  const uint8_t channel_mask = (1 << this->channel_);
  const bool stalled = (stall_reg & channel_mask) != 0;
  const bool drive_fail = (df_reg & channel_mask) != 0;

  // --- Publish states ---
  if (this->stall_sensor_ != nullptr)
    this->stall_sensor_->publish_state(stalled);

  if (this->df_sensor_ != nullptr)
    this->df_sensor_->publish_state(drive_fail);

  // --- Logs ---
  ESP_LOGD(TAG, "Fan %d (0x%02X): stall=%d drive_fail=%d",
           this->channel_ + 1, this->parent_->get_i2c_address(),
           stalled, drive_fail);

  if (stalled)
    ESP_LOGW(TAG, "Fan %d on 0x%02X STALLED!",
             this->channel_ + 1, this->parent_->get_i2c_address());

  if (drive_fail)
    ESP_LOGE(TAG, "Fan %d on 0x%02X DRIVE FAIL!",
             this->channel_ + 1, this->parent_->get_i2c_address());
}


}  // namespace emc2305
}  // namespace esphome
