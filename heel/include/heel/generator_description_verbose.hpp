#ifndef HEEL_GENERATOR_DESCRIPTION_VERBOSE_HDR
#define HEEL_GENERATOR_DESCRIPTION_VERBOSE_HDR

#include <sstream>

#include <heel/model_agora.hpp>
#include <heel/model_features.hpp>
#include <heel/model_knob.hpp>
#include <heel/model_metric.hpp>
#include <heel/model_monitor.hpp>
#include <heel/model_state.hpp>

namespace margot {
namespace heel {

std::stringstream description_verbose(const agora_model& model);
std::stringstream description_verbose(const features_model& model);
std::stringstream description_verbose(const knob_model& model);
std::stringstream description_verbose(const metric_model& model);
std::stringstream description_verbose(const monitor_model& model);
std::stringstream description_verbose(const state_model& model);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_DESCRIPTION_VERBOSE_HDR