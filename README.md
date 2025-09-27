
# ESPHome EMC2305 Fan Controller Component

This is a custom ESPHome external component that adds support for the **EMC2305** 5-channel PWM fan controller over I²C.

> 🔧 **Based on the official [EMC2101](https://github.com/esphome/esphome/blob/dev/esphome/components/emc2101)** component in ESPHome by @ellull, adapted and extended with help by [ChatGPT](https://openai.com/chatgpt) for use with the EMC2305 IC.
---

Refer to the [EMC2305 datasheet](https://www.microchip.com/en-us/product/EMC2305) for full pinout and configuration options.

---

## Features

- Control up to 5 fans per EMC2305 chip via PWM or DAC mode  
- Read fan RPM via tachometer feedback  
- Optional sensors for duty cycle and speed
  - Fan stall detection  
  - Drive fail status  
- compatible with ESPHome’s component system

--

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

# Address (Hex) 
#
# 0x2F 
# 0x4C   
# 0x2C    
# 0x2D   
# 0x2E

emc2305:
  - id: emc1
    i2c_id: i2c_bus
    address: 0x2F
    pwm:
      resolution: 8
      divider: 1
    inverted: false

output:
  - platform: emc2305
    emc2305_id: emc1   # reference the parent chip
    channel: 0          # fan 0
    id: emc1_fan1

sensor:
  - platform: emc2305
    emc2305_id: emc1
    channel: 0
    id: emc1_fan1_sensor
    speed:
      name: "Fan 1 Speed"
    duty_cycle:
      name: "Fan 1 Duty"
    stall:
      name: "Fan 1 Stall"
    drive_fail:
      name: "Fan 1 Drive Fail"
    update_interval: 1s

fan:
  - platform: template
    name: "Fan 1"
    id: fan1
    speed_count: 100   # HA will show 0–100%
    on_speed_set:
      - lambda: |-
          float duty = x / 100.0f;  // convert 0-100% to 0.0-1.0
          id(emc1_fan1).set_level(duty);
    on_turn_off:
      - lambda: |-
          id(emc1_fan1).set_level(0.0f);  // explicitly write 0 to PWM
    on_turn_on:
      - lambda: |-
          id(emc1_fan1).set_level(1.0f / 100.0f); // optionally start at lowest speed
