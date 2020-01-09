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

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& block_node, const block_model& block) {
  // put the basic information in the block node
  block_node.put(tag::name(), block.name);
  if (!block.features.empty()) {
    block_node.put(tag::feature_distance(), to_str(block.features.distance_type));
  }

  // and now also the more complex information
  add_list(block_node, block.monitors, tag::monitors());
  add_list(block_node, block.knobs, tag::knobs());
  add_list(block_node, block.metrics, tag::metrics());
  add_element(block_node, block.agora, tag::agora());
  add_list(block_node, block.features.fields, tag::features());
  add_list(block_node, block.states, tag::states());

  // we omit to parse the operating point list field, since it is not really part of the model
}

}  // namespace heel
}  // namespace margot
