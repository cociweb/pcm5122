#include "process_flow_select.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pcm5122 {

static const char *const TAG = "pcm5122.select";

static const uint8_t PROCESS_FLOW_VALUES[] = {
    PCM51XX_PROC_FLOW_1,
    PCM51XX_PROC_FLOW_2,
    PCM51XX_PROC_FLOW_3,
    PCM51XX_PROC_FLOW_5,
    PCM51XX_PROC_FLOW_7,
};

void ProcessFlowSelect::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Process Flow Select");

  // Build pointer array pointing into stored option strings
  this->option_ptrs_.init(PROCESS_FLOW_OPTION_COUNT);
  for (uint8_t i = 0; i < PROCESS_FLOW_OPTION_COUNT; i++) {
    this->option_ptrs_.push_back(this->stored_options_[i].c_str());
  }
  this->traits.set_options(this->option_ptrs_);

  // Restore saved index from preferences
  size_t restored_index;
  this->pref_ = this->make_entity_preference<size_t>();
  if (this->pref_.load(&restored_index)) {
    if (restored_index < PROCESS_FLOW_OPTION_COUNT) {
      this->publish_state(restored_index);
      this->parent_->set_process_flow(PROCESS_FLOW_VALUES[restored_index]);
      return;
    }
  }

  // No saved preference — publish current hardware state
  uint8_t current_flow = this->parent_->get_process_flow();
  for (uint8_t i = 0; i < PROCESS_FLOW_OPTION_COUNT; i++) {
    if (PROCESS_FLOW_VALUES[i] == current_flow) {
      this->publish_state(i);
      return;
    }
  }

  // Fallback to flow 5
  this->publish_state(3); // index 3 = Flow 5
}

void ProcessFlowSelect::dump_config() {
  ESP_LOGCONFIG(TAG, "PCM5122 Select:");
  LOG_SELECT("  ", "Process Flow", this);
}

void ProcessFlowSelect::control(size_t index) {
  if (index >= PROCESS_FLOW_OPTION_COUNT) {
    ESP_LOGE(TAG, "Invalid process flow index: %zu", index);
    return;
  }

  this->publish_state(index);
  this->pref_.save(&index);
  this->parent_->set_process_flow(PROCESS_FLOW_VALUES[index]);
}

}  // namespace pcm5122
}  // namespace esphome