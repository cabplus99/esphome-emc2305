#include "emc2305.h"
#include "esphome/core/log.h"

namespace esphome {
namespace emc2305 {

static const char *const TAG = "emc2305";

// Register addresses (from datasheet)
static const uint8_t REG_PRODUCT_ID = 0xFD;
static const uint8_t REG_PWM_OUTPUT_CONFIG = 0x2F;
static const uint8_t FAN_BASE_REG[5] = {0x30, 0x40, 0x50, 0x60, 0x70};
static const uint8_t FAN_SETTING = 0x00;
static const uint8_t FAN_TACH_MSB[5] = {0x3E, 0x4E, 0x5E, 0x6E, 0x7E};
static const uint8_t FAN_TACH_LSB[5] = {0x3F, 0x4F, 0x5F, 0x6F, 0x7F};
static const uint8_t FAN_CONFIG[5]   = {0x32, 0x42, 0x52, 0x62, 0x72};
static const uint8_t FAN_RAMP[5]     = {0x36, 0x46, 0x56, 0x66, 0x76};

///////////////////////////////////////////////////////////////////////////
// Setup
///////////////////////////////////////////////////////////////////////////
void Emc2305Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up EMC2305 at 0x%02X...", this->address_);

  uint8_t id = 0;
  if (!this->read_byte(REG_PRODUCT_ID, &id)) {
    ESP_LOGE(TAG, "Failed to read Product ID at 0x%02X", this->address_);
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG, "[0x%02X] Product ID: 0x%02X", this->address_, id);
  if (id != 0x34 && id != 0x35) {
    ESP_LOGW(TAG, "[0x%02X] Unexpected Product ID (expected 0x34/0x35)", this->address_);
  }

  // Enable push-pull mode for PWM outputs
  uint8_t pwm_cfg = 0;
  if (this->read_byte(REG_PWM_OUTPUT_CONFIG, &pwm_cfg)) {
    pwm_cfg |= 0x80;
    this->write_byte(REG_PWM_OUTPUT_CONFIG, pwm_cfg);
    ESP_LOGI(TAG, "[0x%02X] PWM outputs set to Push-Pull", this->address_);
  }

  // Enable tach on all 5 fans
  for (int i = 0; i < 5; i++) {
    // --- FAN_CONFIG ---
    // bit4 = tach enable
    // bits2:0 = tach range (0x07 = max range)
    uint8_t fan_config =
        0x10 |    // TACH enable
        0x07;     // TACH range max

    if (!this->write_byte(FAN_CONFIG[i], fan_config)) {
      ESP_LOGW(TAG, "Failed to set FAN_CONFIG for fan %d", i + 1);
    }

    // --- RAMP / SPIN-UP ---
    // Spin-up register is 0x36/0x46/0x56/0x66/0x76
    if (this->ramp_step_ > 0) {
      if (!this->write_byte(FAN_RAMP[i], this->ramp_step_)) {
        ESP_LOGW(TAG, "Failed to set ramp step for fan %d", i + 1);
      }
    }

    // --- Start stopped ---
    this->set_duty_cycle(i, 0.0f);
  }



  ESP_LOGI(TAG, "[0x%02X] Init complete", this->address_);
}

///////////////////////////////////////////////////////////////////////////
// Dump Config
///////////////////////////////////////////////////////////////////////////
void Emc2305Component::dump_config() {
  ESP_LOGCONFIG(TAG, "EMC2305 at 0x%02X:", this->address_);
  ESP_LOGCONFIG(TAG, "  Mode: %s", dac_mode_ ? "DAC" : "PWM");
  ESP_LOGCONFIG(TAG, "  Resolution: %d bits", pwm_resolution_);
  ESP_LOGCONFIG(TAG, "  Divider: %d", pwm_divider_);
  ESP_LOGCONFIG(TAG, "  Inverted: %s", YESNO(inverted_));
  for (int i = 0; i < 5; i++) {
    ESP_LOGCONFIG(TAG, "  Fan %d duty=%.2f, rpm=%.1f", i + 1, duty_cycle_[i], rpm_[i]);
  }
}

///////////////////////////////////////////////////////////////////////////
// Duty Cycle Control
///////////////////////////////////////////////////////////////////////////
void Emc2305Component::set_duty_cycle(uint8_t channel, float value) {
  if (channel >= 5) {
    ESP_LOGW(TAG, "[0x%02X] Invalid channel: %d", this->address_, channel);
    return;
  }

  if (value < 0.0f) value = 0.0f;
  if (value > 1.0f) value = 1.0f;

  uint8_t duty = static_cast<uint8_t>(value * this->max_output_value_);
  if (inverted_) duty = this->max_output_value_ - duty;

  uint8_t reg = FAN_BASE_REG[channel] + FAN_SETTING;
  if (!this->write_byte(reg, duty)) {
    ESP_LOGE(TAG, "[0x%02X] Failed to set Fan %d duty", this->address_, channel + 1);
    return;
  }

  duty_cycle_[channel] = value;
  ESP_LOGD(TAG, "[0x%02X] Fan %d duty set to %.1f%%", this->address_, channel + 1, value * 100.0f);
}

float Emc2305Component::get_duty_cycle(uint8_t channel) const {
  if (channel >= 5) return NAN;
  return duty_cycle_[channel];
}

///////////////////////////////////////////////////////////////////////////
// Tachometer Reading
///////////////////////////////////////////////////////////////////////////
float Emc2305Component::get_speed(uint8_t channel) {
    uint8_t lsb, msb;
    if (!this->read_byte(FAN_TACH_LSB[channel], &lsb) ||
        !this->read_byte(FAN_TACH_MSB[channel], &msb)) {
        ESP_LOGW(TAG, "Failed to read fan tachometer for channel %d", channel);
        return 0.0f;
    }

    // Tach is 12 bits: MSB[7:0] + LSB[7:4]
    uint16_t tach = ((static_cast<uint16_t>(msb) << 4) | (lsb >> 4)) & 0x0FFF;

    // 0x000 = invalid, 0xFFF = overflow/too slow
    if (tach == 0 || tach == 0x0FFF) {
        return 0.0f;
    }
    uint8_t tach_range_[5] = {0, 0, 0, 0, 0};

    // Optional: per-fan calibration factor
    float calibration_factor_[5] = {2.1f, 2.1f, 2.1f, 2.1f, 2.1f};
  
    // TACH range factor: 2^TR[2:0]
    uint8_t range_bits = tach_range_[channel] & 0x07;
    uint16_t range_factor = 1 << range_bits;

    // Constants
    const float F_OSC = 32000.0f;   // internal tachometer clock (32 kHz)
    const float PULSES_PER_REV = 2.0f; // most 3/4-wire fans = 2 PPR

    // Datasheet formula: RPM = 60 * Fosc / (tach * PPR * 2)
    float rpm = (60.0f * F_OSC * range_factor) / (static_cast<float>(tach) * PULSES_PER_REV * 2.0f);


    return rpm * calibration_factor_[channel];
}




///////////////////////////////////////////////////////////////////////////
// Setup Priority
///////////////////////////////////////////////////////////////////////////
float Emc2305Component::get_setup_priority() const { return setup_priority::HARDWARE; }

}  // namespace emc2305
}  // namespace esphome
