#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include <heel/logger.hpp>
#include <heel/model/metric.hpp>

void margot::heel::validate(metric_model& model) {
  // perform checks on the metric type
  if (model.type.empty()) {
    margot::heel::error("The metric \"", model.name, "\" must have a type");
    throw std::runtime_error("metric model: metric without a type");
  }
  if (model.type.compare("string") == 0) {
    margot::heel::error("The type string for the metric \"", model.name, "\" is not supported");
    throw std::runtime_error("metric model: unsupported type");
  }

  // make sure that if plugin parameters exist, there should be also a plugin name
  if ((!model.prediction_parameters.empty()) && (model.prediction_plugin.empty())) {
    margot::heel::error("The metric \"", model.name, "\" defines plugin parameters, but no plugin");
    throw std::runtime_error("metric model: empty prediction plugin");
  }
}
