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

// forward declaration of the function that actually parse a feature description
margot::heel::feature_model parse_feature_model(const pt::ptree& feature_node);

// this function basically iterates over the features defined in the file and call the appropriate function
// to parse it, appending the new feature to the result vector. Moreover, it parse the string which tells how
// to compute the distance between feature clusters
margot::heel::features_model margot::heel::parse_features(const pt::ptree& block_node) {
  margot::heel::features_model model;

  // set the feature distance type (if any)
  std::string distance_type_str = margot::heel::get(margot::heel::tag::feature_distance(), block_node);
  std::transform(distance_type_str.begin(), distance_type_str.end(), distance_type_str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  if (distance_type_str.empty()) {
    model.distance_type = margot::heel::features_distance_type::NONE;
  } else if (distance_type_str.compare(
                 margot::heel::to_str(margot::heel::features_distance_type::EUCLIDEAN)) == 0) {
    model.distance_type = margot::heel::features_distance_type::EUCLIDEAN;
  } else if (distance_type_str.compare(
                 margot::heel::to_str(margot::heel::features_distance_type::NORMALIZED)) == 0) {
    model.distance_type = margot::heel::features_distance_type::NORMALIZED;
  } else {
    margot::heel::error("Unknown features distance \"", distance_type_str,
                        "\", should be \"euclidean\" or \"normalized\"");
    throw std::runtime_error("features parser: unknown distance");
  }

  // now we need to parse the fields of the features (if any)
  margot::heel::visit_optional(
      margot::heel::tag::features(), block_node,
      [&model](const pt::ptree::value_type& p) { model.fields.emplace_back(parse_feature_model(p.second)); });

  // the list might be full of feature models, but it is better to sort them according to the feature's name
  std::sort(model.fields.begin(), model.fields.end(),
            [](const margot::heel::feature_model& a, const margot::heel::feature_model& b) {
              return a.name < b.name;
            });
  return model;
}

// this is the main function that actually parse a feature
margot::heel::feature_model parse_feature_model(const pt::ptree& feature_node) {
  // fetch the comparison type as string (lowercase)
  std::string comparison_type_str = margot::heel::get(margot::heel::tag::comparison(), feature_node);
  std::transform(comparison_type_str.begin(), comparison_type_str.end(), comparison_type_str.begin(),
                 [](typename std::string::value_type c) { return std::tolower(c); });

  // parse the content to convert to an enum
  margot::heel::distance_comparison_type comparison_type = margot::heel::distance_comparison_type::DONT_CARE;
  if (comparison_type_str.compare(
          margot::heel::to_str(margot::heel::distance_comparison_type::LESS_OR_EQUAL)) == 0) {
    comparison_type = margot::heel::distance_comparison_type::LESS_OR_EQUAL;
  } else if (comparison_type_str.compare(
                 margot::heel::to_str(margot::heel::distance_comparison_type::GREATER_OR_EQUAL)) == 0) {
    comparison_type = margot::heel::distance_comparison_type::GREATER_OR_EQUAL;
  } else if ((!comparison_type_str.empty()) &&
             (comparison_type_str.compare(
                  margot::heel::to_str(margot::heel::distance_comparison_type::DONT_CARE)) != 0)) {
    margot::heel::error("Unknown coparison type \"", comparison_type_str, "\"");
    throw std::runtime_error("features parser: unknown comparison type");
  }

  // construct the model
  return {margot::heel::get(margot::heel::tag::name(), feature_node),
          margot::heel::get(margot::heel::tag::type(), feature_node), comparison_type};
}
