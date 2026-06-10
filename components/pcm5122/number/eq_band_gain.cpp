#include "eq_band_gain.h"
#include "esphome/core/log.h"
#include <cmath>

namespace esphome {
namespace pcm5122 {

static const char *const TAG = "pcm5122.number";

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void EqBandGain::setup() {
  ESP_LOGCONFIG(TAG, "Setting up EQ Band %d Gain Number", this->band_ + 1);

  // Restore saved gain from preferences (RTC memory)
  this->pref_ = this->make_entity_preference<float>();
  float restored_gain;
  if (this->pref_.load(&restored_gain)) {
    ESP_LOGD(TAG, "Band %d: restored gain = %.1f dB", this->band_ + 1, restored_gain);
    // Apply the restored gain to hardware (calculate coefficients + write DSP RAM)
    this->apply_gain_(restored_gain);
    this->publish_state(restored_gain);
    return;
  }

  // No saved preference — initialise with 0 dB and apply identity coefficients
  ESP_LOGD(TAG, "Band %d: no saved gain, defaulting to 0 dB", this->band_ + 1);
  this->apply_gain_(0.0f);
  this->publish_state(0.0f);
}

void EqBandGain::dump_config() {
  ESP_LOGCONFIG(TAG, "PCM5122 EQ Band %d Gain:", this->band_ + 1);
  LOG_NUMBER("  ", "Gain", this);
  ESP_LOGCONFIG(TAG, "    Filter Type: %d", this->filter_type_);
  ESP_LOGCONFIG(TAG, "    Frequency: %u Hz", this->frequency_);
  ESP_LOGCONFIG(TAG, "    Q-Factor: %.2f", this->q_factor_);
}

/* ------------------------------------------------------------------
 * Biquad coefficient calculation — ported from bq_calc.c
 *
 * Formulas from Audio EQ Cookbook (R. Bristow-Johnson) and the TI
 * PPC3/TAS5805M web application (biquad.model.js).
 * Output: standard DSP form  H(z) = (b0+b1·z⁻¹+b2·z⁻²)/(1+a1·z⁻¹+a2·z⁻²)
 * ------------------------------------------------------------------ */
int EqBandGain::calculate_coefficients_(double gain_db, BiquadCoeffs *c) const {
  if (!c || this->frequency_ < 1 || this->q_factor_ <= 0.0) {
    return -1;
  }

  const double freq_hz = (double)this->frequency_;
  const double q = (double)this->q_factor_;
  const double fs = 48000.0;  // Fixed 48 kHz sample rate

  switch (this->filter_type_) {

    /* ------------------------------------------------------------------
     * EQUALIZER (Q FACTOR) — equalizerQFactorCalc() from biquad.model.js.
     * At 0 dB (g=1) the filter is transparent: H(z) = 1.
     * ------------------------------------------------------------------ */
    case BQ_FILTER_EQ_Q_FACTOR: {
      double g = pow(10.0, gain_db / 20.0);
      double t0 = 2.0 * M_PI * freq_hz / fs;
      double cos_t0 = cos(t0);
      double beta = (g >= 1.0) ? t0 / (2.0 * q) : t0 / (2.0 * g * q);

      double a2r = -0.5 * (1.0 - beta) / (1.0 + beta);
      double a1r = (0.5 - a2r) * cos_t0;
      double tmp = (g - 1.0) * (0.25 + 0.5 * a2r);

      c->b0 = 2.0 * (tmp + 0.5);
      c->b1 = -2.0 * a1r;
      c->b2 = 2.0 * (-tmp - a2r);
      c->a1 = -2.0 * a1r;
      c->a2 = -2.0 * a2r;
      break;
    }

    /* ------------------------------------------------------------------
     * Low-pass — lowPassCalc() "VARIABLE Q 2"
     * ------------------------------------------------------------------ */
    case BQ_FILTER_LOW_PASS: {
      double wc = 2.0 * M_PI * freq_hz;
      double k = wc / tan(M_PI * freq_hz / fs);
      double A1_ct = wc / q;
      double wc2 = wc * wc;
      double k2 = k * k;
      double denom = k2 + A1_ct * k + wc2;

      c->b0 = wc2 / denom;
      c->b1 = 2.0 * wc2 / denom;
      c->b2 = wc2 / denom;
      c->a1 = -(2.0 * (k2 - wc2)) / denom;
      c->a2 = -((A1_ct * k - k2 - wc2)) / denom;
      break;
    }

    /* ------------------------------------------------------------------
     * High-pass — highPassCalc() "VARIABLE Q 2"
     * ------------------------------------------------------------------ */
    case BQ_FILTER_HIGH_PASS: {
      double wc = 2.0 * M_PI * freq_hz;
      double k = wc / tan(M_PI * freq_hz / fs);
      double A1_ct = wc / q;
      double wc2 = wc * wc;
      double k2 = k * k;
      double denom = k2 + A1_ct * k + wc2;

      c->b0 = k2 / denom;
      c->b1 = -2.0 * k2 / denom;
      c->b2 = k2 / denom;
      c->a1 = -(2.0 * (k2 - wc2)) / denom;
      c->a2 = -((A1_ct * k - k2 - wc2)) / denom;
      break;
    }

    /* ------------------------------------------------------------------
     * Low-shelf — lowShelfCalc() from biquad.model.js
     * ------------------------------------------------------------------ */
    case BQ_FILTER_LOW_SHELF: {
      double A = sqrt(pow(10.0, gain_db / 20.0));
      double wo = 2.0 * M_PI * freq_hz / fs;
      double cos_wo = cos(wo);
      double sin_wo = sin(wo);
      double alpha = sin_wo / (2.0 * q);
      double sqrtA = sqrt(A);
      double ao = (A + 1.0) + (A - 1.0) * cos_wo + 2.0 * sqrtA * alpha;

      c->b0 = A * ((A + 1.0) - (A - 1.0) * cos_wo + 2.0 * sqrtA * alpha) / ao;
      c->b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cos_wo) / ao;
      c->b2 = A * ((A + 1.0) - (A - 1.0) * cos_wo - 2.0 * sqrtA * alpha) / ao;
      c->a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cos_wo) / ao;
      c->a2 = ((A + 1.0) + (A - 1.0) * cos_wo - 2.0 * sqrtA * alpha) / ao;
      break;
    }

    /* ------------------------------------------------------------------
     * High-shelf — highShelfCalc() from biquad.model.js
     * ------------------------------------------------------------------ */
    case BQ_FILTER_HIGH_SHELF: {
      double A = sqrt(pow(10.0, gain_db / 20.0));
      double wo = 2.0 * M_PI * freq_hz / fs;
      double cos_wo = cos(wo);
      double sin_wo = sin(wo);
      double alpha = sin_wo / (2.0 * q);
      double sqrtA = sqrt(A);
      double ao = (A + 1.0) - (A - 1.0) * cos_wo + 2.0 * sqrtA * alpha;

      c->b0 = A * ((A + 1.0) + (A - 1.0) * cos_wo + 2.0 * sqrtA * alpha) / ao;
      c->b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cos_wo) / ao;
      c->b2 = A * ((A + 1.0) + (A - 1.0) * cos_wo - 2.0 * sqrtA * alpha) / ao;
      c->a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cos_wo) / ao;
      c->a2 = ((A + 1.0) - (A - 1.0) * cos_wo - 2.0 * sqrtA * alpha) / ao;
      break;
    }

    default:
      return -1;
  }

  return 0;
}

void EqBandGain::apply_gain_(float value) {
  double gain_db = (double)value;

  BiquadCoeffs coeffs{};
  int ret = this->calculate_coefficients_(gain_db, &coeffs);

  if (ret == 0) {
    // Convert to PCM5122 register format: b0, b1/2, b2, -a1/2, -a2
    float reg_coeffs[5] = {
      (float)coeffs.b0,           // b0
      (float)(coeffs.b1 * 0.5),   // b1/2
      (float)coeffs.b2,           // b2
      (float)(-coeffs.a1 * 0.5),  // -a1/2
      (float)(-coeffs.a2),        // -a2
    };

    ESP_LOGD(TAG,
             "Band %d: gain=%.1fdB  type=%d  freq=%uHz  Q=%.2f  "
             "coeffs=[b0=%.7f  b1/2=%.7f  b2=%.7f  -a1/2=%.7f  -a2=%.7f]",
             this->band_ + 1, gain_db,
             this->filter_type_, this->frequency_, this->q_factor_,
             reg_coeffs[0], reg_coeffs[1], reg_coeffs[2], reg_coeffs[3], reg_coeffs[4]);

    // Write coefficients to PCM5122 DSP RAM via I2C
    if (!this->parent_->write_bq_coefficients(this->band_, reg_coeffs)) {
      ESP_LOGW(TAG, "Band %d: failed to write coefficients to PCM5122", this->band_ + 1);
    }
  } else {
    ESP_LOGW(TAG, "Band %d: coefficient calculation failed (gain=%.1fdB)",
             this->band_ + 1, gain_db);
  }
}

void EqBandGain::control(float value) {
  // Apply the new gain to hardware
  this->apply_gain_(value);

  // Save to preferences so it is restored after reboot
  this->pref_.save(&value);

  this->publish_state(value);
}

}  // namespace pcm5122
}  // namespace esphome