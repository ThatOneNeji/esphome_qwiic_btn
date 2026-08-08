import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_MODE

from . import CONF_QWIIC_BTN_ID, QwiicBtnComponent, qwiic_btn_ns

DEPENDENCIES = ["qwiic_btn"]

PressMode = qwiic_btn_ns.enum("PressMode")
PRESS_MODES = {
    "MOMENTARY": PressMode.MOMENTARY_MODE,
    "LATCH": PressMode.LATCH_MODE,
}

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(binary_sensor.BinarySensor).extend(
    {
        cv.GenerateID(CONF_QWIIC_BTN_ID): cv.use_id(QwiicBtnComponent),
        cv.Optional(CONF_MODE, default="MOMENTARY"): cv.enum(PRESS_MODES, upper=True),
    }
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    parent = await cg.get_variable(config[CONF_QWIIC_BTN_ID])
    cg.add(parent.set_pressed_binary_sensor(var))
    cg.add(parent.set_press_mode(config[CONF_MODE]))
