import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_CONFIG

from ..audio_dac import CONF_PCM5122_ID, Pcm5122Component, pcm5122_ns

ProcessFlowSelect = pcm5122_ns.class_("ProcessFlowSelect", select.Select, cg.Component)

CONF_PROCESS_FLOW = "process_flow"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_PCM5122_ID): cv.use_id(Pcm5122Component),
        cv.Optional(CONF_PROCESS_FLOW): select.select_schema(
            ProcessFlowSelect,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
    }
)

async def to_code(config):
    pcm5122_component = await cg.get_variable(config[CONF_PCM5122_ID])
    if process_flow_config := config.get(CONF_PROCESS_FLOW):
        s = await select.new_select(
            process_flow_config,
            options=[],
        )
        await cg.register_component(s, process_flow_config)
        await cg.register_parented(s, pcm5122_component)