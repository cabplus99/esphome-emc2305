import esphome.codegen as cg
from esphome.components import output
import esphome.config_validation as cv
from esphome.const import CONF_ID

from .. import CONF_EMC2305_ID, EMC2305_COMPONENT_SCHEMA, emc2305_ns

DEPENDENCIES = ["emc2305"]

EMC2305Output = emc2305_ns.class_("Emc2305Output", output.FloatOutput)

CONFIG_SCHEMA = EMC2305_COMPONENT_SCHEMA.extend(
    {
        cv.Required(CONF_ID): cv.declare_id(EMC2305Output),
        cv.Required("channel"): cv.int_range(min=0, max=4),
    }
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_EMC2305_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_channel(config["channel"]))
    await output.register_output(var, config)

