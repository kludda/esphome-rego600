import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID, CONF_MAX_VALUE, CONF_MIN_VALUE, CONF_STEP
from .. import rego_ns, RegoInterfaceComponent, CONF_HUB_ID, CONF_REGO_VARIABLE, CONF_VALUE_FACTOR, not_zero_or_small # CONF_RETRY_WRITE, 

DEPENDENCIES = ['rego600']

RegoNumber = rego_ns.class_("RegoNumber", number.Number, cg.PollingComponent)
CONFIG_SCHEMA = (
    number.number_schema(RegoNumber) # Changed from _SCHEMA to _schema()
    .extend( 
        {
            cv.GenerateID(CONF_HUB_ID): cv.use_id(RegoInterfaceComponent),
            cv.Required(CONF_REGO_VARIABLE): cv.hex_uint16_t,
            cv.Required(CONF_MIN_VALUE): cv.float_,
            cv.Required(CONF_MAX_VALUE): cv.float_,
            cv.Required(CONF_STEP): cv.positive_float,
            cv.Optional(CONF_VALUE_FACTOR, default=1.): cv.All(cv.float_, not_zero_or_small),
        }
    )
    .extend(cv.polling_component_schema('120s'))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await number.register_number(var, config, min_value=config[CONF_MIN_VALUE], max_value=config[CONF_MAX_VALUE], step=config[CONF_STEP])
    cg.add(var.set_rego_variable(int(config[CONF_REGO_VARIABLE])))
    cg.add(var.set_value_factor(config[CONF_VALUE_FACTOR]))
    paren = await cg.get_variable(config[CONF_HUB_ID])
    cg.add(var.register_hub(paren))
