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

#ifndef HEEL_PARSER_STATE_HDR
#define HEEL_PARSER_STATE_HDR

#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_state.hpp>

namespace margot {
namespace heel {

void parse(state_model& state, const boost::property_tree::ptree& state_node);
void parse(rank_field_model& rank_field, const boost::property_tree::ptree& rank_field_node);
void parse(constraint_model& constraint, const boost::property_tree::ptree& constraint_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_STATE_HDR