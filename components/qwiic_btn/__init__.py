import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID
from esphome.core import TimePeriod

CODEOWNERS = ["@ThatOneNeji"]
DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["binary_sensor", "output"]
MULTI_CONF = True

qwiic_btn_ns = cg.esphome_ns.namespace("qwiic_btn")
QwiicBtnComponent = qwiic_btn_ns.class_(
    "QwiicBtnComponent", cg.Component, i2c.I2CDevice
)

CONF_QWIIC_BTN_ID = "qwiic_btn_id"
CONF_DEBOUNCE_TIME = "debounce_time"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(QwiicBtnComponent),
            cv.Optional(CONF_DEBOUNCE_TIME, default="20ms"): cv.All(
                cv.positive_time_period_milliseconds,
                cv.Range(max=TimePeriod(milliseconds=65535)),
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x6F))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_debounce_time(config[CONF_DEBOUNCE_TIME].total_milliseconds))
