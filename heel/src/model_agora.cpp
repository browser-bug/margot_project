#include <stdexcept>
#include <string>

#include <heel/logger.hpp>
#include <heel/model/agora.hpp>

void margot::heel::validate(margot::heel::agora_model& model) {
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
  }
}
