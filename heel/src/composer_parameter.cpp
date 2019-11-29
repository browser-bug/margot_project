#include <stdexcept>

#include <boost/property_tree/ptree.hpp>

#include <heel/composer_parameter.hpp>
#include <heel/model_parameter.hpp>
#include <heel/parser_tags.hpp>

void margot::heel::compose(boost::property_tree::ptree& parameter_node, const parameter& parameter) {
  switch (parameter.type) {
    case margot::heel::parameter_types::IMMEDIATE:
      parameter_node.put("", parameter.content);
      break;
    case margot::heel::parameter_types::VARIABLE:
      parameter_node.put(parameter.content, parameter.value_type);
      break;
    default:
      throw std::runtime_error("Unable to compse a parameter with an unknown parameter type");
  }
}

void margot::heel::compose(boost::property_tree::ptree& property_node, const pair_property& property) {
  property_node.put(property.key, property.value);
}