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

namespace margot {
namespace heel {

void parse(feature_model& field, const boost::property_tree::ptree& feature_node) {
  // parse the two feature fields
  parse_element(field.name, feature_node, tag::name());
  parse_element(field.type, feature_node, tag::type());

  // now we need to check which is the comparison function of this field
  std::string comparison_fun;
  parse_element(comparison_fun, feature_node, tag::comparison());
  if (is_enum(comparison_fun, distance_comparison_type::LESS_OR_EQUAL)) {
    field.comparison = distance_comparison_type::LESS_OR_EQUAL;
  } else if (is_enum(comparison_fun, distance_comparison_type::GREATER_OR_EQUAL)) {
    field.comparison = distance_comparison_type::GREATER_OR_EQUAL;
  } else if (is_enum(comparison_fun, distance_comparison_type::DONT_CARE)) {
    field.comparison = distance_comparison_type::DONT_CARE;
  } else if (!comparison_fun.empty()) {
    error("Unknown feature comparison \"", comparison_fun, "\" in field \"", field.name, "\"");
    throw std::runtime_error("field parser: unknown feature comparison");
  } else {
    field.comparison = distance_comparison_type::DONT_CARE;
    warning("Automatically set feature comparison \"-\" to field\"", field.name, "\"");
  }
}

}  // namespace heel
}  // namespace margot