import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from . import CONF_QWIIC_BTN_ID, QwiicBtnComponent

DEPENDENCIES = ["qwiic_btn"]

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(binary_sensor.BinarySensor).extend(
    {
        cv.GenerateID(CONF_QWIIC_BTN_ID): cv.use_id(QwiicBtnComponent),
    }
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    parent = await cg.get_variable(config[CONF_QWIIC_BTN_ID])
    cg.add(parent.set_pressed_binary_sensor(var))
