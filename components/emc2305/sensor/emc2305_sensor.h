#pragma once

#include "../emc2305.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace emc2305 {

/// This class exposes the EMC2305 sensors.
class EMC2305Sensor : public PollingComponent {
 public:
  EMC2305Sensor(Emc2305Component *parent) : parent_(parent) {}

  void dump_config() override;
  void update() override;
  float get_setup_priority() const override;

  void set_speed_sensor(sensor::Sensor *sensor) { this->speed_sensor_ = sensor; }
  void set_duty_cycle_sensor(sensor::Sensor *sensor) { this->duty_cycle_sensor_ = sensor; }

  void set_channel(uint8_t channel) { this->channel_ = channel; }
  
  uint8_t get_channel() const { return this->channel_; }

 protected:
  Emc2305Component *parent_;
  sensor::Sensor *speed_sensor_{nullptr};
  sensor::Sensor *duty_cycle_sensor_{nullptr};

  uint8_t channel_;
};

}  // namespace emc2305
}  // namespace esphome
