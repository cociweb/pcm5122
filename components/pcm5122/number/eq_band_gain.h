#pragma once

#include "../pcm5122.h"
#include "esphome/components/number/number.h"
#include "esphome/core/preferences.h"
#include "esphome/core/component.h"

namespace esphome {
namespace pcm5122 {

/* ------------------------------------------------------------------
 * Filter type enum matching bq_calc component values.
 * Maps to string values configured in YAML.
 * ------------------------------------------------------------------ */
enum EqBandFilterType : uint8_t {
  BQ_FILTER_EQ_Q_FACTOR = 0,
  BQ_FILTER_LOW_PASS,
  BQ_FILTER_HIGH_PASS,
  BQ_FILTER_HIGH_SHELF,
  BQ_FILTER_LOW_SHELF,
};

/* ------------------------------------------------------------------
 * Biquad coefficients in standard DSP form.
 *   H(z) = (b0 + b1·z⁻¹ + b2·z⁻²) / (1 + a1·z⁻¹ + a2·z⁻²)
 * ------------------------------------------------------------------ */
struct BiquadCoeffs {
  double b0, b1, b2, a1, a2;
};

/* ------------------------------------------------------------------
 * EQ Band Gain number component
 * One per band — the only runtime-adjustable value for each band.
 * filter_type, frequency, q_factor, min_gain, max_gain are set from
 * YAML config during code generation.
 *
 * Persists gain value across reboots using ESPHome preferences (RTC
 * memory).  On setup, the saved gain is restored and the corresponding
 * biquad coefficients are written to the PCM5122 DSP RAM.
 * ------------------------------------------------------------------ */
class EqBandGain : public number::Number, public Component, public Parented<Pcm5122Component> {
 public:
  void set_band(uint8_t band) { this->band_ = band; }
  void set_filter_type(EqBandFilterType type) { this->filter_type_ = type; }
  void set_frequency(uint16_t freq) { this->frequency_ = freq; }
  void set_q_factor(float q) { this->q_factor_ = q; }
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

 protected:
  uint8_t band_{0};
  EqBandFilterType filter_type_{BQ_FILTER_EQ_Q_FACTOR};
  uint16_t frequency_{1000};
  float q_factor_{1.0f};

  ESPPreferenceObject pref_;

  void control(float value) override;

  /* Apply the given gain to hardware: calculate coefficients + write to DSP. */
  void apply_gain_(float value);

  /* Calculate biquad coefficients using Audio EQ Cookbook formulas. */
  int calculate_coefficients_(double gain_db, BiquadCoeffs *c) const;
};

}  // namespace pcm5122
}  // namespace esphome