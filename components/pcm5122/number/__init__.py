import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    ENTITY_CATEGORY_CONFIG,
    UNIT_DECIBEL
)

from ..audio_dac import CONF_PCM5122_ID, Pcm5122Component, pcm5122_ns

EqBandGain = pcm5122_ns.class_("EqBandGain", number.Number, cg.Component)

# Filter type enum values matching EqBandFilterType in eq_band_gain.h
EqBandFilterTypeEnum = pcm5122_ns.enum("EqBandFilterType", is_class=True)
EQ_BAND_FILTER_TYPE = {
    "Equalizer": EqBandFilterTypeEnum.BQ_FILTER_EQ_Q_FACTOR,
    "Low Pass": EqBandFilterTypeEnum.BQ_FILTER_LOW_PASS,
    "High Pass": EqBandFilterTypeEnum.BQ_FILTER_HIGH_PASS,
    "High Shelf": EqBandFilterTypeEnum.BQ_FILTER_HIGH_SHELF,
    "Low Shelf": EqBandFilterTypeEnum.BQ_FILTER_LOW_SHELF,
}

# Gain range limits (matching PCM51XX digital volume range)
GAIN_MIN = -24
GAIN_MAX = 24


def _validate_band_config(config):
    """Validate band configuration after decibel conversion."""
    min_gain = config.get("min_gain")
    max_gain = config.get("max_gain")

    if max_gain < min_gain:
        raise cv.Invalid(
            f"max_gain ({max_gain}dB) must be >= min_gain ({min_gain}dB)"
        )
    return config


def _band_schema():
    """Generate schema for a single band's EQ configuration.

    YAML example:
        band_1:
            filter_type: "Low Shelf"
            frequency: 63
            q_factor: 1.0
            min_gain: -12dB
            max_gain: 12dB
            gain:
                name: "EQ Band 1 Gain"
    """
    return cv.All(
        cv.Schema(
            {
                cv.Optional("filter_type", default="Peaking EQ"): cv.enum(
                    EQ_BAND_FILTER_TYPE, upper=False
                ),
                cv.Optional("frequency", default=1000): cv.All(
                    cv.int_, cv.int_range(1, 20000)
                ),
                cv.Optional("q_factor", default=1.0): cv.float_,
                cv.Optional("min_gain", default="-12dB"): cv.All(
                    cv.decibel, cv.int_range(GAIN_MIN, GAIN_MAX)
                ),
                cv.Optional("max_gain", default="12dB"): cv.All(
                    cv.decibel, cv.int_range(GAIN_MIN, GAIN_MAX)
                ),
                cv.Optional("gain"): number.number_schema(
                    EqBandGain,
                    entity_category=ENTITY_CATEGORY_CONFIG,
                    unit_of_measurement=UNIT_DECIBEL,
                ),
            }
        ),
        _validate_band_config,
    )


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_PCM5122_ID): cv.use_id(Pcm5122Component),
        cv.Optional("band_1"): _band_schema(),
        cv.Optional("band_2"): _band_schema(),
        cv.Optional("band_3"): _band_schema(),
        cv.Optional("band_4"): _band_schema(),
        cv.Optional("band_5"): _band_schema(),
        cv.Optional("band_6"): _band_schema(),
    }
)


async def to_code(config):
    pcm5122_component = await cg.get_variable(config[CONF_PCM5122_ID])

    for band_num in range(1, 7):
        band_config = config.get(f"band_{band_num}")
        if band_config is None:
            continue

        # Static configuration values (validated by schema)
        filter_type = band_config["filter_type"]    # already validated as C++ enum
        frequency = band_config["frequency"]
        q_factor = band_config["q_factor"]
        min_gain = band_config["min_gain"]
        max_gain = band_config["max_gain"]

        # Create the runtime-adjustable Gain number
        if gain_config := band_config.get("gain"):
            n = await number.new_number(
                gain_config,
                min_value=float(min_gain),
                max_value=float(max_gain),
                step=0.5,
            )
            await cg.register_component(n, gain_config)
            await cg.register_parented(n, pcm5122_component)
            cg.add(n.set_band(band_num - 1))
            cg.add(n.set_filter_type(filter_type))
            cg.add(n.set_frequency(frequency))
            cg.add(n.set_q_factor(q_factor))