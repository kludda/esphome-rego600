import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import CONF_ID
from .. import rego_ns, RegoInterfaceComponent, CONF_HUB_ID, CONF_REGO_VARIABLE

DEPENDENCIES = ['rego600']

RegoButton = rego_ns.class_("RegoButton", button.Button, cg.PollingComponent)
CONFIG_SCHEMA = (
    button.button_schema(RegoButton)  # Changed from _SCHEMA to _schema()
        .extend( 
            {
                cv.GenerateID(): cv.declare_id(RegoButton),
                cv.GenerateID(CONF_HUB_ID): cv.use_id(RegoInterfaceComponent),
                cv.Required(CONF_REGO_VARIABLE): cv.hex_uint16_t,
            }
        )
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await button.register_button(var, config)
    cg.add(var.set_rego_variable(int(config[CONF_REGO_VARIABLE])))
    paren = await cg.get_variable(config[CONF_HUB_ID])
    cg.add(var.register_hub(paren))
