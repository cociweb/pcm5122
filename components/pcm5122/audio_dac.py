import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.components.audio_dac import AudioDac
from esphome import pins

from esphome.const import (
    CONF_BITS_PER_SAMPLE,
    CONF_ID,
    CONF_ENABLE_PIN,
)

CODEOWNERS = ["@sonocotta"]
DEPENDENCIES = ["i2c"]

CONF_ANALOG_GAIN = "analog_gain"
CONF_MIXER_MODE = "mixer_mode"
CONF_VOLUME_MIN = "volume_min"
CONF_VOLUME_MAX = "volume_max"
CONF_PCM5122_ID = "pcm5122_id"
CONF_CLOCK_MODE = "clock_mode"
CONF_DSP = "dsp"
CONF_DE_EMPHASIS = "de_emphasis"
CONF_SOFT_MUTE = "soft_mute"
CONF_AUTO_MUTE_TIME = "auto_mute_time"
CONF_RAMP_STEP = "ramp_step"

pcm5122_ns = cg.esphome_ns.namespace("pcm5122")
Pcm5122Component = pcm5122_ns.class_("Pcm5122Component", AudioDac, cg.Component, i2c.I2CDevice)

MixerMode = pcm5122_ns.enum("MixerMode")
MIXER_MODES = {
    "STEREO"         : MixerMode.STEREO,
    "STEREO_INVERSE" : MixerMode.STEREO_INVERSE,
    "RIGHT"          : MixerMode.RIGHT,
    "LEFT"           : MixerMode.LEFT,
}

Pcm5122ClockMode = pcm5122_ns.enum("Pcm5122ClockMode")
CLOCK_MODES = {
    "AUTO": Pcm5122ClockMode.CLOCK_MODE_AUTO,
    "BCK": Pcm5122ClockMode.CLOCK_MODE_BCK,
}

Pcm5122BitsPerSample = pcm5122_ns.enum("Pcm5122BitsPerSample")
BITS_PER_SAMPLE = {
    16: Pcm5122BitsPerSample.PCM5122_BITS_PER_SAMPLE_16,
    24: Pcm5122BitsPerSample.PCM5122_BITS_PER_SAMPLE_24,
    32: Pcm5122BitsPerSample.PCM5122_BITS_PER_SAMPLE_32,
}

Pcm5122AutoMuteTime = pcm5122_ns.enum("Pcm5122AutoMuteTime")
AUTO_MUTE_TIMES = {
    "21ms": Pcm5122AutoMuteTime.AUTO_MUTE_21MS,
    "106ms": Pcm5122AutoMuteTime.AUTO_MUTE_106MS,
    "213ms": Pcm5122AutoMuteTime.AUTO_MUTE_213MS,
    "533ms": Pcm5122AutoMuteTime.AUTO_MUTE_533MS,
    "1.07s": Pcm5122AutoMuteTime.AUTO_MUTE_1_07S,
    "2.13s": Pcm5122AutoMuteTime.AUTO_MUTE_2_13S,
    "5.33s": Pcm5122AutoMuteTime.AUTO_MUTE_5_33S,
    "10.66s": Pcm5122AutoMuteTime.AUTO_MUTE_10_66S,
}

Pcm5122RampStep = pcm5122_ns.enum("Pcm5122RampStep")
RAMP_STEPS = {
    "4dB": Pcm5122RampStep.RAMP_STEP_4DB,
    "2dB": Pcm5122RampStep.RAMP_STEP_2DB,
    "1dB": Pcm5122RampStep.RAMP_STEP_1DB,
    "0.5dB": Pcm5122RampStep.RAMP_STEP_0_5DB,
}

ANALOG_GAINS = [-6,  0]
_validate_bits = cv.float_with_unit("bits", "bit")

def validate_config(config):
    if config[CONF_ANALOG_GAIN] not in ANALOG_GAINS:
        raise cv.Invalid(f"analog_gain must be one of {ANALOG_GAINS}")
    if (config[CONF_VOLUME_MAX] - config[CONF_VOLUME_MIN]) < 9:
        raise cv.Invalid("volume_max must at least 9db greater than volume_min")
    return config

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Pcm5122Component),
            cv.Optional(CONF_ENABLE_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_ANALOG_GAIN, default="0dB"): cv.All(
                        cv.decibel, cv.one_of(*ANALOG_GAINS)
            ),
            cv.Optional(CONF_MIXER_MODE, default="STEREO"): cv.enum(
                        MIXER_MODES, upper=True
            ),
            cv.Optional(CONF_CLOCK_MODE, default="AUTO"): cv.enum(
                        CLOCK_MODES, upper=True
            ),
            cv.Optional(CONF_BITS_PER_SAMPLE, default="16bit"): cv.All(
                        _validate_bits, cv.enum(BITS_PER_SAMPLE)
            ),
            cv.Optional(CONF_DSP, default={}): cv.Schema(
                {
                    cv.Optional(CONF_DE_EMPHASIS, default=False): cv.boolean,
                    cv.Optional(CONF_SOFT_MUTE, default=True): cv.boolean,
                    cv.Optional(CONF_AUTO_MUTE_TIME, default="21ms"): cv.enum(
                        AUTO_MUTE_TIMES
                    ),
                    cv.Optional(CONF_RAMP_STEP, default="1dB"): cv.enum(
                        RAMP_STEPS
                    ),
                }
            ),
            cv.Optional(CONF_VOLUME_MAX, default="-3dB"): cv.All(
                        cv.decibel, cv.int_range(-103, 24)
            ),
            cv.Optional(CONF_VOLUME_MIN, default="-103dB"): cv.All(
                        cv.decibel, cv.int_range(-103, 24)
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x4D))
    .add_extra(validate_config),
    cv.only_on_esp32,
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    
    if CONF_ENABLE_PIN in config:
        enable = await cg.gpio_pin_expression(config[CONF_ENABLE_PIN])
        cg.add(var.set_enable_pin(enable))
    
    cg.add(var.config_analog_gain(config[CONF_ANALOG_GAIN]))
    cg.add(var.config_mixer_mode(config[CONF_MIXER_MODE]))
    cg.add(var.config_clock_mode(config[CONF_CLOCK_MODE]))
    cg.add(var.config_bits_per_sample(config[CONF_BITS_PER_SAMPLE]))
    cg.add(var.config_dsp_de_emphasis(config[CONF_DSP][CONF_DE_EMPHASIS]))
    cg.add(var.config_dsp_soft_mute(config[CONF_DSP][CONF_SOFT_MUTE]))
    cg.add(var.config_dsp_auto_mute_time(config[CONF_DSP][CONF_AUTO_MUTE_TIME]))
    cg.add(var.config_dsp_ramp_step(config[CONF_DSP][CONF_RAMP_STEP]))
    cg.add(var.config_volume_max(config[CONF_VOLUME_MAX]))
    cg.add(var.config_volume_min(config[CONF_VOLUME_MIN]))
