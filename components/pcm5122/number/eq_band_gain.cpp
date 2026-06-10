#include "eq_band_gain.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pcm5122 {

static const char *const TAG = "pcm5122.number";

void EqBandGain::setup() {
  ESP_LOGCONFIG(TAG, "Setting up EQ Band %d Gain Number", this->band_ + 1);

  // Publish initial state (0 dB default)
  this->publish_state(0.0f);
}

void EqBandGain::dump_config() {
  ESP_LOGCONFIG(TAG, "PCM5122 EQ Band %d Gain:", this->band_ + 1);
  LOG_NUMBER("  ", "Gain", this);
}

void EqBandGain::control(float value) {
  ESP_LOGD(TAG, "EQ Band %d Gain set to %.1f dB (implementation pending)",
           this->band_ + 1, value);
  this->publish_state(value);
}

}  // namespace pcm5122
}  // namespace esphome