/* mARGOt HEEL library
 * Copyright (C) 2018 Davide Gadioli
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

#include <string>

#include <boost/property_tree/ptree.hpp>

#include <heel/composer_state.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_state.hpp>
#include <heel/parser_tags.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& state_node, const state_model& state) {
  // write the basic information about the knob
  state_node.put(tag::name(), state.name);

  // get the correct tag for the direction
  std::string rank_direction_tag;
  switch (state.direction) {
    case rank_direction::MINIMIZE:
      rank_direction_tag = tag::minimize();
      break;
    case rank_direction::MAXIMIZE:
      rank_direction_tag = tag::minimize();
      break;
    case rank_direction::NONE:
      break;
  };

  // get the correct tag for the rank composition type
  std::string rank_combination_tag;
  switch (state.combination) {
    case rank_type::SIMPLE:
      rank_combination_tag = tag::linear_mean();
      break;
    case rank_type::LINEAR:
      rank_combination_tag = tag::linear_mean();
      break;
    case rank_type::GEOMETRIC:
      rank_combination_tag = tag::geometric_mean();
      break;
    case rank_type::NONE:
      break;
  };

  // compose the actual rank
  boost::property_tree::ptree rank_combination_node;
  add_list(rank_combination_node, state.rank_fields, rank_combination_tag);
  state_node.add_child(rank_direction_tag, rank_combination_node);

  // compose the constraint
  add_list(state_node, state.constraints, tag::constraints());
}

void compose(boost::property_tree::ptree& rank_node, const rank_field_model& rank) {
  rank_node.put(rank.name, rank.coefficient);
}

void compose(boost::property_tree::ptree& constraint_node, const constraint_model& constraint) {
  constraint_node.put(tag::name(), constraint.name);
  constraint_node.put(tag::comparison(), to_str(constraint.cfun));
  constraint_node.put(tag::value(), constraint.value);
  constraint_node.put(tag::confidence(), constraint.confidence);
}

}  // namespace heel
}  // namespace margot