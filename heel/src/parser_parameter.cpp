#include <string>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_parameter.hpp>
#include <heel/parser_parameter.hpp>

namespace pt = boost::property_tree;

margot::heel::parameter margot::heel::parse_parameter(const pt::ptree::value_type& parameter_pair) {
  std::string parameter_content = parameter_pair.second.get<std::string>("", "");
  std::string value_type;
  if (parameter_content.empty()) {
    // it's not an immediate, but we need to explore it further to fetch the var name and type
    // NOTE: it should be one value, but in case the user have done some weird stuff, we
    //       want to take only the last element
    for (const auto& pair : parameter_pair.second) {
      parameter_content = pair.first;
      value_type = pair.second.get<std::string>("", "");
    }
  }

  return {
      value_type.empty() ? margot::heel::parameter_types::IMMEDIATE : margot::heel::parameter_types::VARIABLE,
      parameter_content, value_type};
}