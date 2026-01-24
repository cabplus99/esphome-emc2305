#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"

namespace esphome {
namespace emc2305 {

/** Enum listing all DAC conversion rates for the EMC2305.
 *
 * Values correspond to register values from the datasheet.
 */
enum Emc2305DACConversionRate {
  EMC2305_DAC_1_EVERY_16_S,
  EMC2305_DAC_1_EVERY_8_S,
  EMC2305_DAC_1_EVERY_4_S,
  EMC2305_DAC_1_EVERY_2_S,
  EMC2305_DAC_1_EVERY_SECOND,
  EMC2305_DAC_2_EVERY_SECOND,
  EMC2305_DAC_4_EVERY_SECOND,
  EMC2305_DAC_8_EVERY_SECOND,
  EMC2305_DAC_16_EVERY_SECOND,
  EMC2305_DAC_32_EVERY_SECOND,
};

enum Emc2305RampRate {
  EMC2305_RAMP_SLOW   = 0,
  EMC2305_RAMP_MEDIUM = 3,
  EMC2305_RAMP_FAST   = 7,
};

/// This class provides configuration and control for the EMC2305 I²C fan controller.
/// Each EMC2305 chip controls up to 5 fans (PWM/DAC + tachometer feedback).
class Emc2305Component : public Component, public i2c::I2CDevice {
 public:
  /// Set DAC mode (true = DAC, false = PWM).
  void set_dac_mode(bool dac_mode) { 
    this->dac_mode_ = dac_mode; 
    this->max_output_value_ = dac_mode ? 63 : ((1 << pwm_resolution_) - 1);
  }

  /// Set the PWM resolution (bits).
  void set_pwm_resolution(uint8_t resolution) {
    this->pwm_resolution_ = resolution;
    this->max_output_value_ = (1 << resolution) - 1;
  }

  /// Set the PWM divider used for frequency scaling.
  void set_pwm_divider(uint8_t divider) { this->pwm_divider_ = divider; }

  /// Set the DAC conversion rate (updates per second).
  void set_dac_conversion_rate(Emc2305DACConversionRate conversion_rate) {
    this->dac_conversion_rate_ = conversion_rate;
  }
  
  void set_ramp_step(uint8_t step) { this->ramp_step_ = step; }
  void set_ramp_rate(uint8_t rate) { this->ramp_rate_ = rate; }

  /// Invert the polarity of the PWM output.
  void set_inverted(bool inverted) { this->inverted_ = inverted; }

  /// Set the duty cycle for a specific fan channel (0–4).
  void set_duty_cycle(uint8_t channel, float value);

  /// Get the last commanded duty cycle for a specific channel.
  float get_duty_cycle(uint8_t channel) const;

  /// Read the measured tachometer speed (RPM) for a specific channel.
  float get_speed(uint8_t channel);

  /// Framework setup.
  void setup() override;

  /// Framework config dump.
  void dump_config() override;

  /// Setup priority (run after i2c).
  float get_setup_priority() const override;

 protected:
  // Configuration options
  bool dac_mode_{false};
  bool inverted_{false};
  uint8_t max_output_value_{255};
  uint8_t pwm_resolution_{8};
  uint8_t pwm_divider_{1};
  uint8_t ramp_step_{0};
  uint8_t ramp_rate_{0};
  Emc2305DACConversionRate dac_conversion_rate_{EMC2305_DAC_1_EVERY_SECOND};

  // Per-fan state (5 fans per chip)
  float duty_cycle_[5] = {0};  ///< Last set duty cycle (0.0–1.0).
  float rpm_[5] = {0};         ///< Last measured RPM.
};

}  // namespace emc2305
}  // namespace esphome
