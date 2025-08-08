import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_SPEED,
    CONF_CHANNEL,
    ICON_PERCENT,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    UNIT_REVOLUTIONS_PER_MINUTE,
)

from .. import CONF_EMC2305_ID, EMC2305_COMPONENT_SCHEMA, emc2305_ns

DEPENDENCIES = ["emc2305"]

CONF_DUTY_CYCLE = "duty_cycle"
CONF_CHANNEL = "channel"

EMC2305Sensor = emc2305_ns.class_("EMC2305Sensor", cg.PollingComponent)

CONFIG_SCHEMA = EMC2305_COMPONENT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(EMC2305Sensor),
        cv.Required(CONF_CHANNEL): cv.All(cv.int_, cv.Range(min=0, max=4)),
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
    }
).extend(cv.polling_component_schema("60s"))


async def to_code(config):
    paren = await cg.get_variable(config[CONF_EMC2305_ID])
    var = cg.new_Pvariable(config[CONF_ID], paren)
    await cg.register_component(var, config)
    

    cg.add(var.set_channel(config[CONF_CHANNEL]))

    if CONF_SPEED in config:
        sens = await sensor.new_sensor(config[CONF_SPEED])
        cg.add(var.set_speed_sensor(sens))

    if CONF_DUTY_CYCLE in config:
        sens = await sensor.new_sensor(config[CONF_DUTY_CYCLE])
        cg.add(var.set_duty_cycle_sensor(sens))
