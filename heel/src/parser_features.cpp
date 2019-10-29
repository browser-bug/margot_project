#include <algorithm>
#include <stdexcept>
#include <string>

#include <boost/property_tree/ptree.hpp>

#include <heel/logger.hpp>
#include <heel/model/features.hpp>
#include <heel/parser/features.hpp>
#include <heel/parser/utils.hpp>

namespace pt = boost::property_tree;

// this struct is used to store the actual values of a tag
struct tag {
  inline static const std::string features(void) { return "features"; }
  inline static const std::string feature_distance(void) { return "feature_distance"; }
  inline static const std::string name(void) { return "name"; }
  inline static const std::string feature_type(void) { return "type"; }
  inline static const std::string comparison(void) { return "comparison"; }
};

// forward declaration of the function that actually parse a feature description
margot::heel::feature_model parse_feature_model(const pt::ptree& feature_node);

// this function basically iterates over the features defined in the file and call the appropriate function
// to parse it, appending the new feature to the result vector. Moreover, it parse the string which tells how
// to compute the distance between feature clusters
margot::heel::features_model margot::heel::parse_features(const pt::ptree& block_node) {
  margot::heel::features_model model;

  // set the feature distance type (if any)
  std::string distance_type_str = margot::heel::get(tag::feature_distance(), block_node);
  std::transform(distance_type_str.begin(), distance_type_str.end(), distance_type_str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  if (distance_type_str.empty()) {
    model.distance_type = margot::heel::features_distance_type::NONE;
  } else if (distance_type_str.compare("euclidean") == 0) {
    model.distance_type = margot::heel::features_distance_type::EUCLIDEAN;
  } else if (distance_type_str.compare("normalized") == 0) {
    model.distance_type = margot::heel::features_distance_type::NORMALIZED;
  } else {
    margot::heel::error("Unknown features distance \"", distance_type_str,
                        "\", should be \"euclidean\" or \"normalized\"");
    throw std::runtime_error("features parser: unknown distance");
  }

  // now we need to parse the fields of the features (if any)
  margot::heel::visit_optional(tag::features(), block_node, [&model](const pt::ptree::value_type& p) {
    model.fields.emplace_back(parse_feature_model(p.second));
  });

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
  std::string comparison_type_str = margot::heel::get(tag::comparison(), feature_node);
  std::transform(comparison_type_str.begin(), comparison_type_str.end(), comparison_type_str.begin(),
                 [](typename std::string::value_type c) { return std::tolower(c); });

  // parse the content to convert to an enum
  margot::heel::distance_comparison_type comparison_type = margot::heel::distance_comparison_type::DONT_CARE;
  if (comparison_type_str.compare("le") == 0) {
    comparison_type = margot::heel::distance_comparison_type::LESS_OR_EQUAL;
  } else if (comparison_type_str.compare("ge") == 0) {
    comparison_type = margot::heel::distance_comparison_type::GREATER_OR_EQUAL;
  } else if ((!comparison_type_str.empty()) && (comparison_type_str.compare("-") != 0)) {
    margot::heel::error("Unknown coparison type \"", comparison_type_str, "\"");
    throw std::runtime_error("features parser: unknown comparison type");
  }

  // construct the model
  return {margot::heel::get(tag::name(), feature_node), margot::heel::get(tag::feature_type(), feature_node),
          comparison_type};
}
