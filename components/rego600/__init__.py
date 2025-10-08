import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ['@kludda']

#AUTO_LOAD = [ 'binary_sensor', 'sensor', 'number', 'button', 'text_sensor' ] # 'climate', 'switch'
MULTI_CONF = True

DEPENDENCIES = ["uart"]

CONF_HUB_ID = "rego600_id"
CONF_MODEL = "model"
CONF_REGO_VARIABLE = "rego_variable"
CONF_VALUE_FACTOR = "value_factor"

rego_ns = cg.esphome_ns.namespace("rego")
RegoInterfaceComponent = rego_ns.class_("RegoInterfaceComponent", cg.Component)

SMALL_NUMBER = 0.0001
def not_zero_or_small(value):
    if value and (value > SMALL_NUMBER or value < -SMALL_NUMBER):
        return value
    raise cv.Invalid(f"Value factor cannot be zero or close to zero (+-{SMALL_NUMBER})")

CONFIG_SCHEMA = cv.All(
    # cv.require_esphome_version(2022, 3, 0),
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(RegoInterfaceComponent),
            cv.Optional(CONF_MODEL, default="rego600"): cv.string,
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_model(config[CONF_MODEL]))

    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)  # TODO: needed?
