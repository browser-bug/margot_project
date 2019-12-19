#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/logger.hpp>
#include <heel/model_state.hpp>
#include <heel/parser_state.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

void margot::heel::parse(state_model& state, const boost::property_tree::ptree& state_node) {
  // parse the immediate information from the state
  margot::heel::parse_element(state.name, state_node, margot::heel::tag::name());
  margot::heel::parse_list(state.constraints, state_node, margot::heel::tag::constraints());
  state.direction = margot::heel::rank_direction::NONE;
  state.combination = margot::heel::rank_type::NONE;

  // parse the rank fields of the state by guessing the possible combination, therefore it is better to define
  // a lambda function to do so. The idea is to fix a combination of rank type and direction and try to read
  // from it. If we hit, we set the value accordingly;
  const auto parse_rank = [&state, &state_node](const margot::heel::rank_direction direction,
                                                const margot::heel::rank_type combination) {
    const std::string rank_comb = margot::heel::to_str(combination);
    const std::string rank_dir = margot::heel::to_str(direction);
    margot::heel::parse_list(state.rank_fields, state_node, rank_dir + "." + rank_comb);
    if (!state.rank_fields.empty()) {
      state.direction = direction;
      state.combination = combination;
    }
  };
  parse_rank(margot::heel::rank_direction::MINIMIZE, margot::heel::rank_type::GEOMETRIC);
  if (state.rank_fields.empty()) {
    parse_rank(margot::heel::rank_direction::MINIMIZE, margot::heel::rank_type::LINEAR);
  }
  if (state.rank_fields.empty()) {
    parse_rank(margot::heel::rank_direction::MAXIMIZE, margot::heel::rank_type::GEOMETRIC);
  }
  if (state.rank_fields.empty()) {
    parse_rank(margot::heel::rank_direction::MAXIMIZE, margot::heel::rank_type::LINEAR);
  }
  if (state.rank_fields.empty()) {
    margot::heel::warning("Undefined rank in state \"", state.name, "\"");
  }
}

void margot::heel::parse(rank_field_model& rank_field, const boost::property_tree::ptree& rank_field_node) {
  // NOTE: it should be a single key-value item, but we need to make sure to take only the last one
  for (const auto& pair : rank_field_node) {
    rank_field.name = pair.first;
    margot::heel::parse(rank_field.coefficient, pair.second);
  }
}

void margot::heel::parse(constraint_model& constraint, const boost::property_tree::ptree& constraint_node) {
  // set the basic information about the parser
  margot::heel::parse_element(constraint.name, constraint_node, margot::heel::tag::subject());
  margot::heel::parse_element(constraint.value, constraint_node, margot::heel::tag::value());
  margot::heel::parse_element(constraint.confidence, constraint_node, margot::heel::tag::confidence());

  // parse the comparison type
  std::string constraint_fun;
  margot::heel::parse_element(constraint_fun, constraint_node, margot::heel::tag::comparison());
  if (margot::heel::is_enum(constraint_fun, margot::heel::goal_comparison::LESS_OR_EQUAL)) {
    constraint.cfun = margot::heel::goal_comparison::LESS_OR_EQUAL;
  } else if (margot::heel::is_enum(constraint_fun, margot::heel::goal_comparison::GREATER_OR_EQUAL)) {
    constraint.cfun = margot::heel::goal_comparison::GREATER_OR_EQUAL;
  } else if (margot::heel::is_enum(constraint_fun, margot::heel::goal_comparison::LESS)) {
    constraint.cfun = margot::heel::goal_comparison::LESS;
  } else if (margot::heel::is_enum(constraint_fun, margot::heel::goal_comparison::GREATER)) {
    constraint.cfun = margot::heel::goal_comparison::GREATER;
  } else {
    margot::heel::error("Unable to understand comparison function \"", constraint_fun, "\" in constraint \"",
                        constraint.name, "\"");
    throw std::runtime_error("constraint parser: unknown comparison function");
  }
}
