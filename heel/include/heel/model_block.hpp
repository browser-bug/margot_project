#ifndef HEEL_MODEL_BLOCK_HDR
#define HEEL_MODEL_BLOCK_HDR

#include <string>
#include <vector>

#include <heel/model_agora.hpp>
#include <heel/model_features.hpp>
#include <heel/model_knob.hpp>
#include <heel/model_metric.hpp>
#include <heel/model_monitor.hpp>
#include <heel/model_state.hpp>

namespace margot {
namespace heel {

struct block_model {
  std::string name;
  std::vector<monitor_model> monitors;
  std::vector<knob_model> knobs;
  std::vector<metric_model> metrics;
  features_model features;
  agora_model agora;
  std::vector<state_model> states;
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_BLOCK_HDR