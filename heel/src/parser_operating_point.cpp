#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

#include <heel/logger.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/model_features.hpp>
#include <heel/model_knob.hpp>
#include <heel/model_metric.hpp>
#include <heel/model_operating_point.hpp>
#include <heel/parser_operating_point.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

// this struct is used to store the actual values of a tag
struct tag {
  inline static const std::string features(void) { return "features"; }
  inline static const std::string knobs(void) { return "knobs"; }
  inline static const std::string metrics(void) { return "metrics"; }
};

// helper function that retrieves a value from a field of an Operating Point. It complains if it is invalid or
// empty. It basically parse and validates the Operating Point.
margot::heel::operating_point_value parse_op_field(const std::string& tag_name,
                                                   const boost::property_tree::ptree& op_node) {
  // try to get the expected child of the op
  const boost::optional<const boost::property_tree::ptree&> field = op_node.get_child_optional(tag_name);
  if (!field) {
    margot::heel::error("Unable to understand the Operating Point, missing field \"", tag_name, "\"");
    throw std::runtime_error("op parser: missing field");
  }
  margot::heel::operating_point_value result;

  // try to get the result directly from the node (if it has no standard deviation)
  const auto mean_value = field->get<std::string>("", "");
  if (mean_value.empty()) {
    // the value is actually a distribution, let's find out if it is ok
    const auto mean_value_iterator = field->begin();
    if (mean_value_iterator == field->end()) {
      margot::heel::error("Unable to understand the Operating Point, empty field \"", tag_name, "\"");
      throw std::runtime_error("op parser: empty mean");
    } else {
      const auto std_dev_value_iterator = std::next(mean_value_iterator);
      if (std_dev_value_iterator == field->end()) {
        margot::heel::error("Unable to understand the Operating Point, incomplete field \"", tag_name, "\"");
        throw std::runtime_error("op parser: empty standard deviation");
      }
      result.mean = mean_value_iterator->second.get<std::string>("", "");
      result.standard_deviation = std_dev_value_iterator->second.get<std::string>("", "");
      if (result.mean.empty()) {
        margot::heel::error("Unable to understand the Operating Point, empty field \"", tag_name, "\"");
        throw std::runtime_error("op parser: empty mean");
      }
    }
  } else {
    result.mean = mean_value;
  }
  return result;
}

std::vector<margot::heel::operating_point_model> margot::heel::parse_operating_points(
    const boost::property_tree::ptree& op_node, const block_model& block) {
  // we assume that the file is about the given name, let's start parsing them
  std::vector<margot::heel::operating_point_model> result;
  margot::heel::visit_optional(block.name, op_node, [&result, &block](const pt::ptree::value_type& p) {
    // declare the Operating Point model
    margot::heel::operating_point_model op;

    // parse all the features
    std::for_each(block.features.fields.begin(), block.features.fields.end(),
                  [&p, &op](const feature_model& feature) {
                    op.features.emplace_back(parse_op_field(tag::features() + "." + feature.name, p.second));
                  });

    // parse all the knobs
    std::for_each(block.knobs.begin(), block.knobs.end(), [&p, &op](const knob_model& knob) {
      op.knobs.emplace_back(parse_op_field(tag::knobs() + "." + knob.name, p.second));
    });

    // parse all the metrics
    std::for_each(block.metrics.begin(), block.metrics.end(), [&p, &op](const metric_model& metric) {
      op.metrics.emplace_back(parse_op_field(tag::metrics() + "." + metric.name, p.second));
    });

    // append the operating point to the block
    result.emplace_back(op);
  });
  return result;
}
