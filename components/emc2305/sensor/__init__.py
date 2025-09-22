import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor, binary_sensor
from esphome.const import (
    CONF_ID,
    CONF_SPEED,
    UNIT_PERCENT,
    UNIT_REVOLUTIONS_PER_MINUTE,
    STATE_CLASS_MEASUREMENT,
    ICON_PERCENT,
)

from .. import CONF_EMC2305_ID, EMC2305_COMPONENT_SCHEMA, emc2305_ns

CONF_DUTY_CYCLE = "duty_cycle"
CONF_CHANNEL = "channel"
CONF_STALL = "stall"
CONF_DRIVE_FAIL = "drive_fail"

Emc2305Sensor = emc2305_ns.class_("Emc2305Sensor", cg.PollingComponent)

CONFIG_SCHEMA = EMC2305_COMPONENT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(Emc2305Sensor),
        cv.Required(CONF_CHANNEL): cv.int_range(min=0, max=4),
        cv.Optional(CONF_SPEED): sensor.sensor_schema(
            unit_of_measurement=UNIT_REVOLUTIONS_PER_MINUTE,
            accuracy_decimals=2,
            state_class=STATE_CLASS_MEASUREMENT,
            icon="mdi:fan",
        ),
        cv.Optional(CONF_DUTY_CYCLE): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=2,
            state_class=STATE_CLASS_MEASUREMENT,
            icon=ICON_PERCENT,
        ),
        cv.Optional(CONF_STALL): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_DRIVE_FAIL): binary_sensor.binary_sensor_schema(),
    }
).extend(cv.polling_component_schema("60s"))


async def to_code(config):
    parent = await cg.get_variable(config[CONF_EMC2305_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_channel(config[CONF_CHANNEL]))
    await cg.register_component(var, config)

    if CONF_SPEED in config:
        sens = await sensor.new_sensor(config[CONF_SPEED])
        cg.add(var.set_speed_sensor(sens))

    if CONF_DUTY_CYCLE in config:
        sens = await sensor.new_sensor(config[CONF_DUTY_CYCLE])
        cg.add(var.set_duty_cycle_sensor(sens))

    if CONF_STALL in config:
        s = await binary_sensor.new_binary_sensor(config[CONF_STALL])
        cg.add(var.set_stall_sensor(s))

    if CONF_DRIVE_FAIL in config:
        s = await binary_sensor.new_binary_sensor(config[CONF_DRIVE_FAIL])
        cg.add(var.set_df_sensor(s))
