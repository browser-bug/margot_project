#ifndef HEEL_CPP_INTERFACE_CONVERSION_HDR
#define HEEL_CPP_INTERFACE_CONVERSION_HDR

#include <string>
#include <vector>

#include <heel/generator_utils.hpp>
#include <heel/model_features.hpp>
#include <heel/model_knob.hpp>
#include <heel/model_metric.hpp>

namespace margot {
namespace heel {

// this is an helper struct to write the declaration of input/output parameters
struct cpp_parameters {
  inline static std::string signature(const std::vector<feature_model>& fields,
                                      const std::vector<knob_model>& knobs,
                                      const std::vector<metric_model>& metrics) {
    std::string result;
    if (!fields.empty()) {
      result = margot::heel::join(fields.begin(), fields.end(), ", ",
                                  [](const feature_model& field) { return field.type + " " + field.name; });
      result += ", ";
    }
    result += margot::heel::join(knobs.begin(), knobs.end(), ", ",
                                 [](const knob_model& knob) { return knob.type + " " + knob.name; });
    result += ", ";
    result += margot::heel::join(metrics.begin(), metrics.end(), ", ",
                                 [](const metric_model& metric) { return metric.type + " " + metric.name; });
    return result;
  }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CPP_INTERFACE_CONVERSION_HDR