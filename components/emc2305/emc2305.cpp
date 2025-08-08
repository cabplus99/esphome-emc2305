#include "esphome/core/log.h"
#include "emc2305.h"

namespace esphome {
namespace emc2305 {

static const char *const TAG = "EMC2305";

static const uint8_t EMC2305_CHIP_ID = 0x16;      // EMC2305 default device id from part id
static const uint8_t EMC2305_ALT_CHIP_ID = 0x28;  // EMC2305 alternate device id from part id

// EMC2305 registers from the datasheet. We only define what we use.
static const uint8_t EMC2305_REGISTER_DAC_CONV_RATE = 0x04;      // DAC convesion rate config
static const uint8_t EMC2305_REGISTER_CONFIG = 0x03;             // configuration register
static const uint8_t EMC2305_REGISTER_TACH_LSB = 0x46;           // Tach RPM data low byte
static const uint8_t EMC2305_REGISTER_TACH_MSB = 0x47;           // Tach RPM data high byte
static const uint8_t EMC2305_REGISTER_FAN_CONFIG = 0x4A;         // General fan config register
static const uint8_t EMC2305_REGISTER_FAN_SETTING = 0x4C;        // Fan speed for non-LUT settings
static const uint8_t EMC2305_REGISTER_PWM_FREQ = 0x4D;           // PWM frequency setting
static const uint8_t EMC2305_REGISTER_PWM_DIV = 0x4E;            // PWM frequency divisor
static const uint8_t EMC2305_REGISTER_WHOAMI = 0xFD;             // Chip ID register

// EMC2305 configuration bits from the datasheet. We only define what we use.

// Determines the funcionallity of the ALERT/TACH pin.
// 0 (default): The ALERT/TECH pin will function as an open drain, active low interrupt.
// 1: The ALERT/TECH pin will function as a high impedance TACH input. This may require an
// external pull-up resistor to set the proper signaling levels.
static const uint8_t EMC2305_ALT_TCH_BIT = 1 << 2;

// Determines the FAN output mode.
// 0 (default): PWM output enabled at FAN pin.
// 1: DAC output enabled at FAN ping.
static const uint8_t EMC2305_DAC_BIT = 1 << 4;

// Overrides the CLK_SEL bit and uses the Frequency Divide Register to determine
// the base PWM frequency. It is recommended that this bit be set for maximum PWM resolution.
// 0 (default): The base clock frequency for the PWM is determined by the CLK_SEL bit.
// 1 (recommended): The base clock that is used to determine the PWM frequency is set by the
// Frequency Divide Register
static const uint8_t EMC2305_CLK_OVR_BIT = 1 << 2;

// Sets the polarity of the Fan output driver.
// 0 (default): The polarity of the Fan output driver is non-inverted. A '00h' setting will
// correspond to a 0% duty cycle or a minimum DAC output voltage.
// 1: The polarity of the Fan output driver is inverted. A '00h' setting will correspond to a
// 100% duty cycle or a maximum DAC output voltage.
static const uint8_t EMC2305_POLARITY_BIT = 1 << 4;

float Emc2305Component::get_setup_priority() const { return setup_priority::HARDWARE; }

void Emc2305Component::setup() {
  // make sure we're talking to the right chip
  uint8_t chip_id = reg(EMC2305_REGISTER_WHOAMI).get();
  if ((chip_id != EMC2305_CHIP_ID) && (chip_id != EMC2305_ALT_CHIP_ID)) {
    ESP_LOGE(TAG, "Wrong chip ID %02X", chip_id);
    this->mark_failed();
    return;
  }

  // Configure EMC2305
  i2c::I2CRegister config = reg(EMC2305_REGISTER_CONFIG);
  config |= EMC2305_ALT_TCH_BIT;
  if (this->dac_mode_) {
    config |= EMC2305_DAC_BIT;
  }
  if (this->inverted_) {
    config |= EMC2305_POLARITY_BIT;
  }

  if (this->dac_mode_) {  // DAC mode configurations
    // set DAC conversion rate
    reg(EMC2305_REGISTER_DAC_CONV_RATE) = this->dac_conversion_rate_;
  } else {  // PWM mode configurations
    // set PWM divider
    reg(EMC2305_REGISTER_FAN_CONFIG) |= EMC2305_CLK_OVR_BIT;
    reg(EMC2305_REGISTER_PWM_DIV) = this->pwm_divider_;

    // set PWM resolution
    reg(EMC2305_REGISTER_PWM_FREQ) = this->pwm_resolution_;
  }
}

void Emc2305Component::dump_config() {
  ESP_LOGCONFIG(TAG, "Emc2305 component:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
  }
  ESP_LOGCONFIG(TAG, "  Mode: %s", this->dac_mode_ ? "DAC" : "PWM");
  if (this->dac_mode_) {
    ESP_LOGCONFIG(TAG, "  DAC Conversion Rate: %X", this->dac_conversion_rate_);
  } else {
    ESP_LOGCONFIG(TAG,
                  "  PWM Resolution: %02X\n"
                  "  PWM Divider: %02X",
                  this->pwm_resolution_, this->pwm_divider_);
  }
  ESP_LOGCONFIG(TAG, "  Inverted: %s", YESNO(this->inverted_));
}

void Emc2305Component::set_duty_cycle(float value) {
  uint8_t duty_cycle = remap(value, 0.0f, 1.0f, (uint8_t) 0, this->max_output_value_);
  ESP_LOGD(TAG, "Setting duty fan setting to %02X", duty_cycle);
  if (!this->write_byte(EMC2305_REGISTER_FAN_SETTING, duty_cycle)) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
    this->status_set_warning();
    return;
  }
}

float Emc2305Component::get_duty_cycle(uint8_t channel) {
  const uint8_t duty_reg = EMC2305_REGISTER_FAN_SETTING + channel;
  uint8_t duty_cycle;
  if (!this->read_byte(duty_reg, &duty_cycle)) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
    this->status_set_warning();
    return NAN;
  }
  return remap<float>(static_cast<float>(duty_cycle), 0.0f, static_cast<float>(this->max_output_value_), 0.0f, 1.0f);
}


float Emc2305Component::get_speed(uint8_t channel) {
  const uint8_t lsb_reg = EMC2305_REGISTER_TACH_LSB + channel * 2;
  const uint8_t msb_reg = EMC2305_REGISTER_TACH_MSB + channel * 2;
  uint8_t lsb, msb;
  if (!this->read_byte(lsb_reg, &lsb) || !this->read_byte(msb_reg, &msb)) {
    ESP_LOGE(TAG, ESP_LOG_MSG_COMM_FAIL);
    this->status_set_warning();
    return NAN;
  }
  uint16_t tach = (msb << 8) | lsb;
  return tach == 0xFFFF ? 0.0f : 5400000.0f / tach;
}

}  // namespace emc2305
}  // namespace esphome
