#include <boost/property_tree/ptree.hpp>

#include <heel/composer_agora.hpp>
#include <heel/composer_block.hpp>
#include <heel/composer_feature_fields.hpp>
#include <heel/composer_knob.hpp>
#include <heel/composer_metric.hpp>
#include <heel/composer_monitor.hpp>
#include <heel/composer_state.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_block.hpp>
#include <heel/model_features.hpp>
#include <heel/parser_tags.hpp>

void margot::heel::compose(boost::property_tree::ptree& block_node, const block_model& block) {
  // put the basic information in the block node
  block_node.put(margot::heel::tag::name(), block.name);
  if (!block.features.empty()) {
    block_node.put(margot::heel::tag::feature_distance(), margot::heel::to_str(block.features.distance_type));
  }

  // and now also the more complex information
  margot::heel::add_list(block_node, block.monitors, margot::heel::tag::monitors());
  margot::heel::add_list(block_node, block.knobs, margot::heel::tag::knobs());
  margot::heel::add_list(block_node, block.metrics, margot::heel::tag::metrics());
  margot::heel::add_element(block_node, block.agora, margot::heel::tag::agora());
  margot::heel::add_list(block_node, block.features.fields, margot::heel::tag::features());
  margot::heel::add_list(block_node, block.states, margot::heel::tag::states());

  // we omit to parse the operating point list field, since it is not really part of the model
}