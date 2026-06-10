#pragma once

#include "../pcm5122.h"
#include "esphome/components/number/number.h"
#include "esphome/core/preferences.h"
#include "esphome/core/component.h"

namespace esphome {
namespace pcm5122 {

/* ------------------------------------------------------------------
 * EQ Band Gain number component
 * One per band — the only runtime-adjustable value for each band.
 * filter_type, q_factor, min_gain, max_gain are set from YAML config.
 * Range: min_gain – max_gain dB, step 0.5 dB
 * ------------------------------------------------------------------ */
class EqBandGain : public number::Number, public Component, public Parented<Pcm5122Component> {
 public:
  void set_band(uint8_t band) { this->band_ = band; }
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

 protected:
  uint8_t band_{0};
  void control(float value) override;
};

}  // namespace pcm5122
}  // namespace esphome