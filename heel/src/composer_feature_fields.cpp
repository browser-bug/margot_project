#include <algorithm>
#include <utility>

#include <boost/property_tree/ptree.hpp>

#include <heel/composer_feature_fields.hpp>
#include <heel/composer_parameter.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_features.hpp>
#include <heel/parser_tags.hpp>

void margot::heel::compose(boost::property_tree::ptree& field_node, const feature_model& field) {
  // write the basic information about the feature field
  field_node.put(margot::heel::tag::name(), field.name);
  field_node.put(margot::heel::tag::type(), field.type);
  field_node.put(margot::heel::tag::comparison(), margot::heel::to_str(field.comparison));
}