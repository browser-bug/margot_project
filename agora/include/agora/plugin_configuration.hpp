#ifndef PLUGIN_CONFIGURATION_HPP
#define PLUGIN_CONFIGURATION_HPP

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <utility>

#include "agora/agora_properties.hpp"

namespace agora {

enum class PluginType { DOE, Model, Cluster, Prediction };

struct PluginConfiguration
{
  PluginConfiguration() {}
  PluginConfiguration(const std::string &name, const application_id &application_id) : name(name), app_id(application_id) {}
  PluginConfiguration(const std::string &name, const application_id &application_id, const std::string &metric_name,
                      const int &iteration_number)
      : name(metric_name + "_" + name), app_id(application_id), metric_name(metric_name), iteration_number(iteration_number)
  {}

  // get the properties list with a environmental file compatible format
  inline std::string print_properties() const
  {
    std::stringstream content;
    for (const auto &pair : this->properties)
      content << pair.first << "=\"" << pair.second << "\"\n";
    return content.str();
  }

  // TODO: checks if the config list is valid or not
  // bool validate_properties() const {
  //}

  // this map represents a {key,value} sorted list of the configuration file
  std::map<std::string, std::string> properties;

  // the plugin configuration file_name
  std::string name;

  application_id app_id;

  // this gets assigned only if we have a configuration for a Model plugin type
  std::string metric_name;
  int iteration_number;
};

} // namespace agora

#endif // PLUGIN_CONFIGURATION_HPP
