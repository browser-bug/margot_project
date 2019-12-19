#include <boost/property_tree/ptree.hpp>

#include <heel/model_parameter.hpp>
#include <heel/parser_parameter.hpp>
#include <heel/parser_utils.hpp>

void margot::heel::parse(parameter& parameter, const boost::property_tree::ptree& parameter_node) {
  parameter.type = margot::heel::parameter_types::IMMEDIATE;
  margot::heel::parse(parameter.content, parameter_node);
  if (parameter.content.empty()) {
    // it's not an immediate, but we need to explore it further to fetch the var name and type
    // NOTE: it should be one value, but in case the user have done some weird stuff, we
    //       want to take only the last element
    for (const auto& pair : parameter_node) {
      parameter.content = pair.first;
      margot::heel::parse(parameter.value_type, pair.second);
    }
    parameter.type = margot::heel::parameter_types::VARIABLE;
  }
}

void margot::heel::parse(pair_property& property, const boost::property_tree::ptree& property_node) {
  for (const auto& pair : property_node) {
    property.key = pair.first;
    margot::heel::parse(property.value, pair.second);
  }
}
