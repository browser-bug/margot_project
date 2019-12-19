#include <algorithm>
#include <stdexcept>
#include <string>

#include <boost/property_tree/ptree.hpp>

#include <heel/logger.hpp>
#include <heel/model_features.hpp>
#include <heel/parser_features.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

void margot::heel::parse(feature_model& field, const boost::property_tree::ptree& feature_node) {
  // parse the two feature fields
  margot::heel::parse_element(field.name, feature_node, margot::heel::tag::name());
  margot::heel::parse_element(field.type, feature_node, margot::heel::tag::type());

  // now we need to check which is the comparison function of this field
  std::string comparison_fun;
  margot::heel::parse_element(comparison_fun, feature_node, margot::heel::tag::comparison());
  if (margot::heel::is_enum(comparison_fun, margot::heel::distance_comparison_type::LESS_OR_EQUAL)) {
    field.comparison = margot::heel::distance_comparison_type::LESS_OR_EQUAL;
  } else if (margot::heel::is_enum(comparison_fun,
                                   margot::heel::distance_comparison_type::GREATER_OR_EQUAL)) {
    field.comparison = margot::heel::distance_comparison_type::GREATER_OR_EQUAL;
  } else if (margot::heel::is_enum(comparison_fun, margot::heel::distance_comparison_type::DONT_CARE)) {
    field.comparison = margot::heel::distance_comparison_type::DONT_CARE;
  } else if (!comparison_fun.empty()) {
    margot::heel::error("Unknown feature comparison \"", comparison_fun, "\" in field \"", field.name, "\"");
    throw std::runtime_error("field parser: unknown feature comparison");
  } else {
    field.comparison = margot::heel::distance_comparison_type::DONT_CARE;
    margot::heel::warning("Automatically set feature comparison \"-\" to field\"", field.name, "\"");
  }
}
