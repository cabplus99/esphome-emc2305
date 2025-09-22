#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "../emc2305.h"

namespace esphome {
namespace emc2305 {

class Emc2305Sensor : public PollingComponent {
 public:
  void set_parent(Emc2305Component *parent) { this->parent_ = parent; }
  void set_channel(uint8_t channel) { this->channel_ = channel; }

  // Normal sensors
  void set_speed_sensor(sensor::Sensor *s) { this->speed_sensor_ = s; }
  void set_duty_cycle_sensor(sensor::Sensor *s) { this->duty_cycle_sensor_ = s; }

  // Binary sensors
  void set_stall_sensor(binary_sensor::BinarySensor *s) { this->stall_sensor_ = s; }
  void set_df_sensor(binary_sensor::BinarySensor *s) { this->df_sensor_ = s; }

  void dump_config() override;
  void update() override;
  float get_setup_priority() const override;

 protected:
  Emc2305Component *parent_{nullptr};
  uint8_t channel_{0};

  // Sensors
  sensor::Sensor *speed_sensor_{nullptr};
  sensor::Sensor *duty_cycle_sensor_{nullptr};

  // Binary sensors
  binary_sensor::BinarySensor *stall_sensor_{nullptr};
  binary_sensor::BinarySensor *df_sensor_{nullptr};
};

}  // namespace emc2305
}  // namespace esphome
