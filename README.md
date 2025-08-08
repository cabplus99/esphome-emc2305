# ESPHome EMC2305 Fan Controller Component

This is a custom ESPHome external component that adds support for the **EMC2305** 5-channel PWM fan controller over I²C.

> 🔧 **Based on the official [EMC2101](https://github.com/esphome/esphome/blob/dev/esphome/components/emc2101)** component in ESPHome, adapted and extended with help by [ChatGPT](https://openai.com/chatgpt) for use with the EMC2305 IC.

---

## Features

- Control up to **5 independent PWM fan outputs** per EMC2305 chip
- Support for **RPM monitoring** for connected fans
- Support for **multiple EMC2305 devices** on the same I²C bus
- Designed for full integration into **Home Assistant** via ESPHome

---

Refer to the [EMC2305 datasheet](https://www.microchip.com/en-us/product/EMC2305) for full pinout and configuration options.

---

## YAML Configuration (Example)

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/cabplus99/esphome-emc2305.git
      ref: main
    components: [emc2305]

i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true

emc2305:
  - id: fan_controller_1
    address: 0x2F

output:
  - platform: emc2305
    emc2305_id: fan_controller_1
    channel: 0
    id: fan0_pwm
  - platform: emc2305
    emc2305_id: fan_controller_1
    channel: 1
    id: fan1_pwm
