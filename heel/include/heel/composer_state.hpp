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

#ifndef HEEL_COMPOSER_STATE_HDR
#define HEEL_COMPOSER_STATE_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_state.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& state_node, const state_model& state);
void compose(boost::property_tree::ptree& rank_node, const rank_field_model& rank);
void compose(boost::property_tree::ptree& constraint_node, const constraint_model& constraint);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_COMPOSER_STATE_HDR