#include <string>

#include <boost/property_tree/ptree.hpp>

#include <heel/composer_state.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_state.hpp>
#include <heel/parser_tags.hpp>

void margot::heel::compose(boost::property_tree::ptree& state_node, const state_model& state) {
  // write the basic information about the knob
  state_node.put(margot::heel::tag::name(), state.name);

  // get the correct tag for the direction
  std::string rank_direction_tag;
  switch (state.direction) {
    case margot::heel::rank_direction::MINIMIZE:
      rank_direction_tag = margot::heel::tag::minimize();
      break;
    case margot::heel::rank_direction::MAXIMIZE:
      rank_direction_tag = margot::heel::tag::minimize();
      break;
    case margot::heel::rank_direction::NONE:
      break;
  };

  // get the correct tag for the rank composition type
  std::string rank_combination_tag;
  switch (state.combination) {
    case margot::heel::rank_type::SIMPLE:
      rank_combination_tag = margot::heel::tag::linear_mean();
      break;
    case margot::heel::rank_type::LINEAR:
      rank_combination_tag = margot::heel::tag::linear_mean();
      break;
    case margot::heel::rank_type::GEOMETRIC:
      rank_combination_tag = margot::heel::tag::geometric_mean();
      break;
    case margot::heel::rank_type::NONE:
      break;
  };

  // compose the actual rank
  boost::property_tree::ptree rank_combination_node;
  margot::heel::add_list(rank_combination_node, state.rank_fields, rank_combination_tag);
  state_node.add_child(rank_direction_tag, rank_combination_node);

  // compose the constraint
  margot::heel::add_list(state_node, state.constraints, margot::heel::tag::constraints());
}

void margot::heel::compose(boost::property_tree::ptree& rank_node, const rank_field_model& rank) {
  rank_node.put(rank.name, rank.coefficient);
}

void margot::heel::compose(boost::property_tree::ptree& constraint_node, const constraint_model& constraint) {
  constraint_node.put(margot::heel::tag::name(), constraint.name);
  constraint_node.put(margot::heel::tag::comparison(), margot::heel::to_str(constraint.cfun));
  constraint_node.put(margot::heel::tag::value(), constraint.value);
  constraint_node.put(margot::heel::tag::confidence(), constraint.confidence);
}