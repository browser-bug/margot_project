#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

#include <heel/model_knob.hpp>
#include <heel/parser_knob.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

void margot::heel::parse(knob_model& knob, const boost::property_tree::ptree& knob_node) {
  // parse the esy part: the knob name and type
  margot::heel::parse_element(knob.name, knob_node, margot::heel::tag::name());
  margot::heel::parse_element(knob.type, knob_node, margot::heel::tag::type());

  // now we try to parse the knob values (they are optional)
  margot::heel::parse_list(knob.values, knob_node, margot::heel::tag::values());
  if (knob.values.empty()) {
    // if we already have a list of values it is ok, otherwise we need to compute them
    const auto& range_node = knob_node.get_child_optional(margot::heel::tag::range());
    if (range_node) {
      margot::heel::compute_range(knob.values, *range_node, knob.type);
    }
  }
}
