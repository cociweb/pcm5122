#pragma once

#include "../pcm5122.h"
#include "esphome/components/select/select.h"
#include "esphome/core/preferences.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"  // For FixedVector

namespace esphome {
namespace pcm5122 {

static constexpr uint8_t PROCESS_FLOW_OPTION_COUNT = 5;

class ProcessFlowSelect : public select::Select, public Component, public Parented<Pcm5122Component> {
public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::AFTER_CONNECTION; }

protected:
  ESPPreferenceObject pref_;

  // Pointers into stored_options_
  FixedVector<const char *> option_ptrs_;

  // Storage for actual string data (must persist for lifetime)
  std::string stored_options_[PROCESS_FLOW_OPTION_COUNT] = {
    "Flow 1 (FIR Interpolation)",
    "Flow 2 (Low-latency IIR)",
    "Flow 3 (High-attenuation FIR)",
    "Flow 5 (Parametric EQ)",
    "Flow 7 (Ringing-less FIR)",
  };

  void control(size_t index) override;
};

}  // namespace pcm5122
}  // namespace esphome
