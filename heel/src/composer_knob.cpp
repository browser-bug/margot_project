#include <boost/property_tree/ptree.hpp>

#include <heel/composer_knob.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_knob.hpp>
#include <heel/parser_tags.hpp>

void margot::heel::compose(boost::property_tree::ptree& knob_node, const knob_model& knob) {
  // write the basic information about the knob
  knob_node.put(margot::heel::tag::name(), knob.name);
  knob_node.put(margot::heel::tag::type(), knob.type);

  // write the values filed (we should have already computed any range tag)
  margot::heel::add_list(knob_node, knob.values, margot::heel::tag::values());
}