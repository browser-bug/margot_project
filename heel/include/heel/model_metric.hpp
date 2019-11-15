#ifndef HEEL_MODEL_METRIC_HDR
#define HEEL_MODEL_METRIC_HDR

#include <cstdint>
#include <string>
#include <vector>

#include <heel/model_monitor.hpp>
#include <heel/model_parameter.hpp>

namespace margot {
namespace heel {

struct metric_model {
  std::string name;
  std::string type;
  bool distribution;
  std::string prediction_plugin;
  std::string monitor_name;
  std::size_t inertia;  // zero means no reaction
  std::vector<pair_property> prediction_parameters;
};

// this function validates a metric model to enforce a correct configuration of the application
void validate(metric_model& model, const std::vector<monitor_model>& monitors);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_METRIC_HDR