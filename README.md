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

#| Address (Hex) | Address (Decimal) |
#| ------------- | ----------------- |
#| `0x2F`        | 47                |
#| `0x4C`        | 76                |
#| `0x2C`        | 44                |
#| `0x2D`        | 45                |
#| `0x2E`        | 46                |

i2c:
  sda: GPIO21
  scl: GPIO22
  scan: true

emc2305:
  - id: emc1
    address: 0x2F
    pwm:
      resolution: 8

output:
  - platform: emc2305
    id: fan0_pwm
    channel: 0
    emc2305_id: emc1

sensor:
  - platform: emc2305
    emc2305_id: emc1
    channel: 0
    speed:
      name: "Fan 1 RPM"
    duty_cycle:
      name: "Fan 1 Duty Cycle"

