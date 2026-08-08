import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_ID

from . import CONF_QWIIC_BTN_ID, QwiicBtnComponent, qwiic_btn_ns

DEPENDENCIES = ["qwiic_btn"]

QwiicBtnLEDOutput = qwiic_btn_ns.class_("QwiicBtnLEDOutput", output.FloatOutput)

CONFIG_SCHEMA = output.FLOAT_OUTPUT_SCHEMA.extend(
    {
        cv.Required(CONF_ID): cv.declare_id(QwiicBtnLEDOutput),
        cv.GenerateID(CONF_QWIIC_BTN_ID): cv.use_id(QwiicBtnComponent),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    parent = await cg.get_variable(config[CONF_QWIIC_BTN_ID])
    cg.add(var.set_parent(parent))
    await output.register_output(var, config)
