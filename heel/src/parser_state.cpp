/* mARGOt HEEL library
 * Copyright (C) 2018 Politecnico di Milano
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

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

namespace margot {
namespace heel {

void parse(state_model& state, const boost::property_tree::ptree& state_node) {
  // parse the immediate information from the state
  parse_element(state.name, state_node, tag::name());
  parse_list(state.constraints, state_node, tag::constraints());
  state.direction = rank_direction::NONE;
  state.combination = rank_type::NONE;

  // parse the rank fields of the state by guessing the possible combination, therefore it is better to define
  // a lambda function to do so. The idea is to fix a combination of rank type and direction and try to read
  // from it. If we hit, we set the value accordingly;
  const auto parse_rank = [&state, &state_node](const rank_direction direction, const rank_type combination) {
    const std::string rank_comb = to_str(combination);
    const std::string rank_dir = to_str(direction);
    parse_list(state.rank_fields, state_node, rank_dir + "." + rank_comb);
    if (!state.rank_fields.empty()) {
      state.direction = direction;
      state.combination = combination;
    }
  };
  parse_rank(rank_direction::MINIMIZE, rank_type::GEOMETRIC);
  if (state.rank_fields.empty()) {
    parse_rank(rank_direction::MINIMIZE, rank_type::LINEAR);
  }
  if (state.rank_fields.empty()) {
    parse_rank(rank_direction::MAXIMIZE, rank_type::GEOMETRIC);
  }
  if (state.rank_fields.empty()) {
    parse_rank(rank_direction::MAXIMIZE, rank_type::LINEAR);
  }
  if (state.rank_fields.empty()) {
    warning("Undefined rank in state \"", state.name, "\"");
  }
}

void parse(rank_field_model& rank_field, const boost::property_tree::ptree& rank_field_node) {
  // NOTE: it should be a single key-value item, but we need to make sure to take only the last one
  for (const auto& pair : rank_field_node) {
    rank_field.name = pair.first;
    parse(rank_field.coefficient, pair.second);
  }
}

void parse(constraint_model& constraint, const boost::property_tree::ptree& constraint_node) {
  // set the basic information about the parser
  parse_element(constraint.name, constraint_node, tag::subject());
  parse_element(constraint.value, constraint_node, tag::value());
  parse_element(constraint.confidence, constraint_node, tag::confidence());

  // parse the comparison type
  std::string constraint_fun;
  parse_element(constraint_fun, constraint_node, tag::comparison());
  if (is_enum(constraint_fun, goal_comparison::LESS_OR_EQUAL)) {
    constraint.cfun = goal_comparison::LESS_OR_EQUAL;
  } else if (is_enum(constraint_fun, goal_comparison::GREATER_OR_EQUAL)) {
    constraint.cfun = goal_comparison::GREATER_OR_EQUAL;
  } else if (is_enum(constraint_fun, goal_comparison::LESS)) {
    constraint.cfun = goal_comparison::LESS;
  } else if (is_enum(constraint_fun, goal_comparison::GREATER)) {
    constraint.cfun = goal_comparison::GREATER;
  } else {
    error("Unable to understand comparison function \"", constraint_fun, "\" in constraint \"",
          constraint.name, "\"");
    throw std::runtime_error("constraint parser: unknown comparison function");
  }
}

}  // namespace heel
}  // namespace margot