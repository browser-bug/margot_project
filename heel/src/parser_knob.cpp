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

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

#include <heel/model_knob.hpp>
#include <heel/parser_knob.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

namespace margot {
namespace heel {

void parse(knob_model& knob, const boost::property_tree::ptree& knob_node) {
  // parse the esy part: the knob name and type
  parse_element(knob.name, knob_node, tag::name());
  parse_element(knob.type, knob_node, tag::type());

  // now we try to parse the knob values (they are optional)
  parse_list(knob.values, knob_node, tag::values());
  if (knob.values.empty()) {
    // if we already have a list of values it is ok, otherwise we need to compute them
    const auto& range_node = knob_node.get_child_optional(tag::range());
    if (range_node) {
      compute_range(knob.values, *range_node, knob.type);
    }
  }
}

}  // namespace heel
}  // namespace margot
