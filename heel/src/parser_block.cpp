#include <stdexcept>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/logger.hpp>
#include <heel/parser_agora.hpp>
#include <heel/parser_block.hpp>
#include <heel/parser_features.hpp>
#include <heel/parser_knob.hpp>
#include <heel/parser_metric.hpp>
#include <heel/parser_monitor.hpp>
#include <heel/parser_state.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

void margot::heel::parse(block_model& block, const boost::property_tree::ptree& block_node) {
  // parse the name, metrics and knobs of the block
  margot::heel::parse_element(block.name, block_node, margot::heel::tag::name());
  margot::heel::parse_list(block.monitors, block_node, margot::heel::tag::monitors());
  margot::heel::parse_list(block.knobs, block_node, margot::heel::tag::knobs());
  margot::heel::parse_list(block.metrics, block_node, margot::heel::tag::metrics());

  // parsing the application features is more complicated, since it is an optional section and it
  // contains an enum, that we have to parse and understand
  std::string feature_distance;
  margot::heel::parse_element(feature_distance, block_node, margot::heel::tag::feature_distance());
  if (margot::heel::is_enum(feature_distance, margot::heel::features_distance_type::EUCLIDEAN)) {
    block.features.distance_type = margot::heel::features_distance_type::EUCLIDEAN;
  } else if (margot::heel::is_enum(feature_distance, margot::heel::features_distance_type::NORMALIZED)) {
    block.features.distance_type = margot::heel::features_distance_type::NORMALIZED;
  } else if (!feature_distance.empty()) {
    margot::heel::error("Unknown feature distance \"", feature_distance, "\" in block \"", block.name, "\"");
    throw std::runtime_error("block parser: unknown feature distance");
  } else {
    block.features.distance_type = margot::heel::features_distance_type::NONE;
  }
  margot::heel::parse_list(block.features.fields, block_node, margot::heel::tag::features());

  // now we can parse the remainder of the block fields
  margot::heel::parse_element(block.agora, block_node, margot::heel::tag::agora());
  margot::heel::parse_list(block.states, block_node, margot::heel::tag::states());

  // the operating points are parsed in a different time, since they are no really part of the model
}