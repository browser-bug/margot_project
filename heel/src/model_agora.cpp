#include <stdexcept>
#include <string>
#include <algorithm>

#include <heel/logger.hpp>
#include <heel/model/agora.hpp>

void margot::heel::validate(margot::heel::agora_model& model, const std::vector<metric_model>& metrics,
                            const std::vector<knob_model>& knobs) {
  // the validation of this model is useful only if agora is enabled in the configuration file
  if (model.enabled) {
    // at first check if the user provided at least a broker url and a qos
    if (model.url.empty()) {
      margot::heel::error("To use Agora it is required the url of the MQTT broker");
      throw std::runtime_error("agora model: empty broker url");
    }
    if (model.qos.empty()) {
      margot::heel::error("To use Agora it is required the set the communication level of QoS");
      throw std::runtime_error("agora model: empty connection qos");
    }
    if ((model.qos.compare("0") != 0) && (model.qos.compare("1") != 0) && (model.qos.compare("2") != 0)) {
      margot::heel::error("Unknown MQTT QoS level \"", model.qos,
                          "\", should be an integer number in interval (0,2)");
      throw std::runtime_error("agora model: unknown qos level");
    }

    // then, we need to check if the user set the name of the required plugin
    if (model.doe_plugin.empty()) {
      margot::heel::error("To use Agora it is required the set the name of the plugin that performs the doe");
      throw std::runtime_error("agora model: empty doe plugin");
    }
    if (model.clustering_plugin.empty()) {
      margot::heel::error(
          "To use Agora it is required the set the name of the plugin that performs the clustering");
      throw std::runtime_error("agora model: empty clustering plugin");
    }

    // moreover, there should be at least one metric to model it with agora at runtime
    if (metrics.empty()) {
      margot::heel::error("Using Agora is meaningful if there is at least one metric");
      throw std::runtime_error("agora error: no metric to predict");
    }

    // then, we need to make sure that all the metrics are observed at runtime
    if (std::any_of(metrics.begin(), metrics.end(),
                    [](const metric_model& metric) { return metric.monitor_name.empty(); })) {
      margot::heel::error(
          "At least one metric is not observed by a monitor at runtime, unable to use Agora to derive the "
          "application knowledge");
      throw std::runtime_error("agora error: metric not observed");
    }

    // then, we need to ensure that all the metrics specify a plugin to generate the application knoledge
    if (std::any_of(metrics.begin(), metrics.end(),
                    [](const metric_model& metric) { return metric.prediction_plugin.empty(); })) {
      margot::heel::error("At least one metric has no plugin to predict its performance");
      throw std::runtime_error("agora error: metric not predicted");
    }

    // finally, we need to ensure that all the knobs have a non-empty range of values
    if (std::any_of(knobs.begin(), knobs.end(), [](const knob_model& knob) { return knob.values.empty(); })) {
      margot::heel::error("At least one knob has no range of values, it is impossible to explore");
      throw std::runtime_error("agora error: unknown knob range");
    }
  }
}
