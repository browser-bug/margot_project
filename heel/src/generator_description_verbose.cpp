#include <sstream>

#include <heel/generator/description_verbose.hpp>
#include <heel/generator/utils.hpp>
#include <heel/model/monitor.hpp>

std::stringstream margot::heel::description_verbose(const margot::heel::monitor_model& model) {
  // declare the stream that will hold the monitor synthetic description
  std::stringstream d;

  // provide the most boring information regarding its class
  d << "Monitor \"" << model.cpp_identifier << "\"" << std::endl;
  d << "\tClass name: " << model.spec.class_name << std::endl;
  d << "\tHeader path: " << model.spec.header_path << std::endl;
  d << "\tValue type: " << model.spec.value_type << std::endl;

  // provide the information about the logged measures
  if (!model.requested_statistics.empty()) {
    d << "\tLogging: [";
    d << margot::heel::join(model.requested_statistics.cbegin(), model.requested_statistics.cend(), ", ",
                            [](const std::string& property) { return property; });
    d << "]" << std::endl;
  }

  // provide the information about the constructor
  d << "\tConstructor (";
  d << margot::heel::join(model.initialization_parameters.cbegin(), model.initialization_parameters.cend(),
                          ", ", [](const margot::heel::parameter& parameter) { return parameter.content; });
  d << ")" << std::endl;

  // provide the information about the start method (if any)
  if (!model.spec.start_method_name.empty()) {
    d << "\tStart: " << model.spec.start_method_name << "(";
    d << margot::heel::join(model.start_parameters.cbegin(), model.start_parameters.cend(), ", ",
                            [](const margot::heel::parameter& parameter) { return parameter.content; });
    d << ")" << std::endl;
  }

  // provide the information about the start method (if any)
  if (!model.spec.stop_method_name.empty()) {
    d << "\tStop: " << model.spec.stop_method_name << "(";
    d << margot::heel::join(model.stop_parameters.cbegin(), model.stop_parameters.cend(), ", ",
                            [](const margot::heel::parameter& parameter) { return parameter.content; });
    d << ")" << std::endl;
  }

  return d;
}