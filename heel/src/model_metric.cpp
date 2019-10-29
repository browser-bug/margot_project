#include <algorithm>
#include <stdexcept>
#include <vector>

#include <heel/logger.hpp>
#include <heel/model/metric.hpp>
#include <heel/model/monitor.hpp>

void margot::heel::validate(metric_model& model, const std::vector<monitor_model>& monitors) {
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

  // now we need to be sure that if a metric is observed by a monitor, the monitor exists
  if (!model.monitor_name.empty()) {
    if (!std::any_of(monitors.cbegin(), monitors.cend(), [&model](const monitor_model& monitor) {
          return model.monitor_name.compare(monitor.name) == 0;
        })) {
      margot::heel::error("The metric \"", model.name, "\" is observed by the non-existent monitor \"",
                          model.monitor_name, "\"");
      throw std::runtime_error("metric model: non-existent monitor");
    }
  }
}
