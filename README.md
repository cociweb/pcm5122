# ESPHome Component for PCM5122 DAC

The PCM5122 is a high-performance audio DAC (Digital-to-Analog Converter) designed for portable and automotive audio applications. This ESPHome external component provides easy integration of the PCM5122 DAC with ESP32-based systems for professional audio output.

# Why

I've created a [HiFi-ESP32-Plus](https://sonocotta.com/hifi-esp32-and-hifi-esp32s3/) and [Amped-ESP32-Plus](https://sonocotta.com/amped-esp32/) boards, that uses this DAC, and I wanted to integrate this DAC into the Home Assistant. Even though it is a fairly old DAC, and TI doesn't update they tooling to support it properly, it is still great chip for the price 

## Usage: PCM5122 Component from GitHub

This component requires ESPHome version 2025.7.0 or later.

The following YAML can be used so ESPHome accesses the component files:

```yaml
external_components:
  - source: github://sonocotta/esphome-pcm5122@main
```

## Overview

The PCM5122 is a versatile audio DAC that communicates with the ESP32 via I2C interface (control) and I2S (audio). This component allows control of the DAC's features including digital volume adjustment, mute control, analog gain, mixer controls, DSP presets, mute/ramp tuning, clock configuration and power management through Home Assistant or direct ESPHome automation.

### Key Specifications

- **Interface**: I2C (Two-wire interface) and I2S (Digital Audio)
- **Digital Volume Range**: -103 dB to +24 dB in 0.5 dB steps
- **Volume Control**: 0-100% via ESPHome (internally remapped to dB range)
- **Analog Gain**: Configurable (-6 dB or 0 dB) for 1V or 2V RMS output levels
- **Mute Control**: Register-based (always available) and optional GPIO-based mute
- **Clocking**: Automatic clock configuration or BCK as PLL reference
- **Audio Format**: I2S format with configurable sample width
- **DSP Controls**: DSP program preset, de-emphasis, auto-mute time and internal volume ramp step
- **Sound Presets**: High-level presets for common use cases
- **Default I2C Address**: 0x4D (7-bit)
- **Power Modes**: Play mode (active audio) and Sleep mode (low power)

## Component Features

The PCM5122 ESPHome component provides the following capabilities:

- **Initialize PCM5122 DAC** - Configure and initialize the audio codec on startup
- **Enable/Disable DAC** - Place the DAC into Play mode or low-power Sleep mode via optional switch
- **Volume Control** - Set and get volume level from 0-100% (remapped to -103 dB to +24 dB register range)
- **Analog Gain Control** - Set desired output voltage level
- **Mute Control** - Mute and unmute the DAC via I2C register and optional GPIO pin
- **Power Management** - Enable Deep Sleep mode for low-power operation
- **Volume Range Configuration** - Define maximum and minimum volume limits in dB
- **Basic Mixer control** - Stereo (default), Stereo (inverted channels), Left or Right channel only 
- **Clock and I2S Format Configuration** - Select clock handling and bits per sample
- **Soft Mute and Volume Ramping** - Reduce pops and clicks during stream start/stop and volume changes
- **DSP and Sound Presets** - Configure PCM5122 DSP program and high-level audio profiles

## Hardware Requirements

### I2C Interface

The PCM5122 requires I2C communication:
- **SDA Pin**: Data line (configurable)
- **SCL Pin**: Clock line (configurable)

### Optional GPIO Enable Pin

For hardware-based mute control via GPIO:
- **Mute Pin**: GPIO pin for direct mute control (optional, when XSMT pin is wired to it)
  - `HIGH` = Unmute (audio enabled)
  - `LOW` = Mute (audio disabled)
  - Can also be controlled programmatically via DAC Enable Switch

## YAML Configuration

### Basic Audio DAC Configuration

```yaml
audio_dac:
  - platform: pcm5122
```

### Complete Configuration Example

```yaml
i2c:
  sda: GPIO21
  scl: GPIO27
  id: i2c_bus

audio_dac:
  - platform: pcm5122
    i2c_id: i2c_bus
    enable_pin: GPIO13
    sound_preset: hifi
    mixer_mode: STEREO
    clock_mode: AUTO
    bits_per_sample: 16bit
```

### Configuration Variables

#### audio_dac

- **platform** (*Required*): `pcm5122`
- **i2c_id** (*Optional*): ID of the I2C bus. Leave unset if only one I2C bus is defined.
- **address** (*Optional*): I2C address of the DAC. Defaults to `0x4D` (7-bit address).
- **enable_pin** (*Optional*): GPIO pin used to control DAC mute pin (if connected to XSMT pin).
- **sound_preset** (*Optional*): High-level preset. Defaults to `flat`.
- **analog_gain** (*Optional*): Analog gain in dB. Valid values: `0dB` or `-6dB`. If omitted, it is selected by `sound_preset`.
  - `0dB`: Use for 2V RMS output signal
  - `-6dB`: Use for 1V RMS output signal
- **volume_max** (*Optional*): Maximum volume level in dB. Valid range: `-103dB` to `24dB`. If omitted, it is selected by `sound_preset`. The conservative default from `flat` is `-3dB`.
- **volume_min** (*Optional*): Minimum volume level in dB. Valid range: `-103dB` to `24dB`. If omitted, it is selected by `sound_preset`.
- **mixer_mode** (*Optional*): Channel routing mode. Defaults to `STEREO`.
- **clock_mode** (*Optional*): PCM5122 clock configuration. Defaults to `AUTO`.
- **bits_per_sample** (*Optional*): I2S audio word length. Defaults to `16bit`.
- **dsp** (*Optional*): Low-level DSP configuration block. Values set here override the selected `sound_preset`.

### Allowed Values

#### `sound_preset`

High-level presets configure `analog_gain`, `volume_max`, `volume_min` and the `dsp` block unless those options are explicitly set.

- `flat`: Neutral default profile. Uses the PCM5122 default DSP program and conservative `volume_max: -3dB`.
- `hifi`: General music profile with `analog_gain: -6dB`, `volume_max: -3dB`, high attenuation DSP program, slower mute timing and `0.5dB` ramp steps.
- `speech`: Voice/TTS profile with lower maximum level and low-latency DSP program.
- `small_speaker`: Safer profile for small speakers with lower output level and ringing-less low latency DSP program.
- `night`: Quiet profile with lower maximum volume and slower mute/ramp behavior.

Preset defaults:

| `sound_preset` | `analog_gain` | `volume_max` | `volume_min` | `dsp.preset` | `dsp.auto_mute_time` | `dsp.ramp_step` |
| --- | --- | --- | --- | --- | --- | --- |
| `flat` | `0dB` | `-3dB` | `-103dB` | `flat` | `21ms` | `1dB` |
| `hifi` | `-6dB` | `-3dB` | `-80dB` | `high_attenuation` | `106ms` | `0.5dB` |
| `speech` | `-6dB` | `-6dB` | `-60dB` | `low_latency` | `21ms` | `1dB` |
| `small_speaker` | `-6dB` | `-9dB` | `-70dB` | `ringingless_low_latency` | `106ms` | `0.5dB` |
| `night` | `-6dB` | `-12dB` | `-70dB` | `ringingless_low_latency` | `213ms` | `0.5dB` |

Explicit YAML values override preset defaults:

```yaml
audio_dac:
  - platform: pcm5122
    sound_preset: hifi
    volume_max: -6dB
    dsp:
      ramp_step: 1dB
```

#### `analog_gain`

- `0dB`: 2V RMS output.
- `-6dB`: 1V RMS output.

#### `mixer_mode`

- `STEREO`: Left input to left output, right input to right output.
- `STEREO_INVERSE`: Swapped/inverted stereo routing.
- `LEFT`: Left channel routed to both outputs.
- `RIGHT`: Right channel routed to both outputs.

#### `clock_mode`

- `AUTO`: Use PCM5122 automatic clock divider configuration.
- `BCK`: Force BCK as the PLL reference source. This is useful when no MCLK is supplied and automatic detection is unstable.

#### `bits_per_sample`

- `16bit`
- `24bit`
- `32bit`

Match this to the I2S output configuration. `16bit` is the safest default for ESP-IDF based ESPHome audio.

#### I2S `sample_rate`

Use `48000` as the recommended default sample rate for ESP32/ESP-IDF audio output:

```yaml
speaker:
  - platform: i2s_audio
    sample_rate: 48000
```

The PCM5122 supports 44.1 kHz audio, but `44100` is more sensitive to the ESP32 I2S clock source, divider accuracy and the complete audio pipeline. It may work on some boards, but `48000` is the safer choice when the goal is crackle-free playback.

Use `44100` only when all of the following are true:

- The source material is natively 44.1 kHz and you want to avoid resampling.
- The ESP32 board has stable I2S clocking in your configuration.
- The output has been tested with the selected `clock_mode`, `bits_per_sample` and speaker/resampler chain.

If crackling appears at `44100`, switch back to `48000` and keep `clock_mode: AUTO` first. If the board has no MCLK and clock detection is unstable, also test `clock_mode: BCK`.

#### `volume_max` and `volume_min`

- Valid range: `-103dB` to `24dB`
- `volume_max` must be at least `9dB` greater than `volume_min`
- Avoid positive `volume_max` values unless the whole signal path has enough headroom

### DSP Configuration

The `dsp` block exposes PCM5122 internal DSP and ramp controls.

```yaml
audio_dac:
  - platform: pcm5122
    sound_preset: hifi
    dsp:
      preset: high_attenuation
      soft_mute: true
      de_emphasis: false
      auto_mute_time: 106ms
      ramp_step: 0.5dB
```

#### `dsp.preset`

Selects the PCM5122 DSP program.

- `flat`: Default PCM5122 DSP program.
- `fir_deemphasis`: FIR interpolation with de-emphasis.
- `low_latency`: Low-latency IIR program.
- `high_attenuation`: High attenuation filter program.
- `ringingless_low_latency`: Ringing-less low latency FIR program.

#### `dsp.soft_mute`

- `true`: Enable PCM5122 digital auto-mute control for both channels.
- `false`: Disable the digital auto-mute enable bits.

Default is selected by `sound_preset`; all built-in presets enable it.

#### `dsp.de_emphasis`

- `true`: Enable PCM5122 de-emphasis.
- `false`: Disable de-emphasis.

Leave this `false` for normal modern audio streams. Use `true` only when playing material that requires de-emphasis.

#### `dsp.auto_mute_time`

Controls the PCM5122 zero-detect auto-mute timing.

- `21ms`
- `106ms`
- `213ms`
- `533ms`
- `1.07s`
- `2.13s`
- `5.33s`
- `10.66s`

Longer values can reduce start/stop pops, but delay auto-mute behavior.

#### `dsp.ramp_step`

Controls the PCM5122 internal volume ramp step for normal and emergency ramps.

- `4dB`
- `2dB`
- `1dB`
- `0.5dB`

Smaller steps are smoother but take longer.

## Switches

An optional switch can be configured to control the DAC enable/disable state in Home Assistant.

## Volume Control

Volume is controlled as a percentage (0-100%) which is internally remapped to the configured DAC digital volume range. The `flat` preset uses `-103dB` to `-3dB`; other presets may use a narrower range. Volume changes are ramped to reduce audible steps and pops.

### Setting Volume in YAML

```yaml
media_player:
  - platform: i2s_audio
    # ... other configuration
    volume: 0.8  # 80% volume
```

### Volume Control via Automations

```yaml
automation:
  - trigger:
      platform: state
      entity_id: input_number.audio_volume
    action:
      service: media_player.volume_set
      data:
        entity_id: media_player.my_speaker
        volume_level: "{{ trigger.to_state.state | float / 100 }}"
```

## Mute Control

The PCM5122 supports two types of mute control:

### I2C Register-based Mute

Mute is controlled via the PCM5122 register (Register 0x03):
- Setting bits enable/disable mute on both channels
- Works regardless of GPIO configuration

### Optional GPIO Mute Pin

If a GPIO mute pin connected to the XSMT pin:
- Provides hardware-level mute control
- Works in parallel with register-based mute
- `HIGH` = Unmute (enabled)
- `LOW` = Mute (disabled)

### Mute Control in YAML

```yaml
automation:
  - trigger:
      platform: homeassistant
      event: call_service
      service: media_player.mute_volume
    action:
      - service: media_player.mute_volume
        data:
          entity_id: media_player.my_speaker
          is_volume_muted: true
```

## Integration with I2S Audio

The PCM5122 DAC is typically used with the ESPHome I2S Audio component:

```yaml
i2s_audio:
  - i2s_lrclk_pin: GPIO25
    i2s_bclk_pin: GPIO26
    i2s_dout_pin: GPIO27

media_player:
  - platform: i2s_audio
    name: "My Audio Player"
    dac: pcm5122_dac
    speaker: my_speaker

speaker:
  - platform: i2s_audio
    name: "Speaker"
    dac_type: external
    i2s_audio_id: i2s_audio
```

## Power Modes

### Play Mode (Enabled)

When the DAC is enabled:
- Normal audio operation
- Audio can be produced at configured volume
- Current consumption: ~100-150 mA typical

### Sleep Mode (Deep Sleep)

When the DAC is disabled:
- Reduced power consumption
- Audio output disabled
- Current consumption: < 50 µA typical

## Complete Example YAML

Can be found in the example for [ESP32](/components/pcm5122/yaml/esp32-idf-media-player.yaml) and [ESP32-S3](/components/pcm5122/yaml/esp32s3-idf-media-player.yaml)

## Troubleshooting

### DAC Not Initializing

1. **Check I2C Connection**
   ```bash
   esphome logs components/pcm5122/yaml/idf-media-player.yaml
   ```
   Look for I2C initialization messages and any I2C errors.

2. **Verify I2C Address**
   - Default address is `0x4D` (7-bit)
   - Check hardware configuration if using a different address

3. **Check GPIO Pins**
   - Enable pin is now optional - DAC will work without it
   - Verify pull-up resistors on SDA/SCL lines (typically 4.7k ohms)
   - Ensure I2C pins are not used for other purposes

### No Audio Output

1. **Check Mute Status**
   - Ensure DAC is not muted via register or GPIO
   - Verify speaker is properly connected

2. **Verify I2S Configuration**
   - Ensure I2S audio pins are correctly configured
   - Check I2S clock and data line connections
   - Ensure `bits_per_sample` matches the I2S output format
   - Prefer `sample_rate: 48000` for ESP32/ESP-IDF playback

3. **Volume Level**
   - Confirm volume is not set to 0%
   - Check that volume_min/volume_max are within valid range

4. **Clock Mode**
   - Try `clock_mode: BCK` if the board does not provide MCLK and `AUTO` is unstable
   - Keep `clock_mode: AUTO` for the default automatic clock divider configuration

### Crackling, Pops or Clicks

1. **Use a Conservative Preset**
   ```yaml
   audio_dac:
     - platform: pcm5122
       sound_preset: hifi
   ```

2. **Lower Maximum Volume**
   ```yaml
   volume_max: -6dB
   ```

3. **Use Smoother Ramp Steps**
   ```yaml
   dsp:
     ramp_step: 0.5dB
   ```

4. **Increase Auto-Mute Time**
   ```yaml
   dsp:
     auto_mute_time: 106ms
   ```

5. **Try BCK Clock Reference**
   ```yaml
   clock_mode: BCK
   ```

6. **Avoid 44.1 kHz Until Stable**
   ```yaml
   speaker:
     - platform: i2s_audio
       sample_rate: 48000
   ```

### I2C Communication Errors

1. **Enable Debug Logging**
   ```yaml
   logger:
     level: DEBUG
   ```

2. **Check Voltage Levels**
   - I2C bus should be pulled to 3.3V (for ESP32)
   - Add 4.7k ohm pull-up resistors if missing (pins have no PULLUP support)

## Component Files

The PCM5122 component consists of:

- `pcm5122.h` - Component header file with class definitions
- `pcm5122.cpp` - Component implementation
- `audio_dac.py` - ESPHome integration for YAML configuration
- `binary_sensor.py` - Optional binary sensors for fault detection
- `switch/__init__.py` - Optional switches for DAC control
- `pcm5122_cfg.h` - Configuration and register definitions

## License

This component is licensed under GPLv3

## References

- [PCM5122 Datasheet](https://www.ti.com/product/PCM5122)
- [ESPHome Audio DAC Component](https://esphome.io/components/audio_dac/index.html)
- [ESPHome I2S Audio](https://esphome.io/components/i2s_audio/index.html)
