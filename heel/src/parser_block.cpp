#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/parser_agora.hpp>
#include <heel/parser_block.hpp>
#include <heel/parser_features.hpp>
#include <heel/parser_knob.hpp>
#include <heel/parser_metric.hpp>
#include <heel/parser_monitor.hpp>
#include <heel/parser_state.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

// this struct is used to store the actual values of a tag
struct tag {
  inline static const std::string name(void) { return "name"; }
  inline static const std::string blocks(void) { return "blocks"; }
};

// forward declaration of the function that actually parse a block description
margot::heel::block_model parse_block_model(const pt::ptree& block_node);

// this function basically iterates over the blocks defined in the file and call the appropriate function
// to parse it, appending the new block to the result vector.
std::vector<margot::heel::block_model> margot::heel::parse_blocks(
    const boost::property_tree::ptree& application_node) {
  std::vector<margot::heel::block_model> result;
  margot::heel::visit_optional(tag::blocks(), application_node, [&result](const pt::ptree::value_type& p) {
    result.emplace_back(parse_block_model(p.second));
  });
  return result;
}

margot::heel::block_model parse_block_model(const pt::ptree& block_node) {
  return {margot::heel::get(tag::name(), block_node), margot::heel::parse_monitors(block_node),
          margot::heel::parse_knobs(block_node),      "",
          margot::heel::parse_metrics(block_node),    "",
          margot::heel::parse_features(block_node),   margot::heel::parse_agora(block_node),
          margot::heel::parse_states(block_node)};
}
