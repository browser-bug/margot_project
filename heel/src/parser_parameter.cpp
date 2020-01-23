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

#include <boost/property_tree/ptree.hpp>

#include <heel/model_parameter.hpp>
#include <heel/parser_parameter.hpp>
#include <heel/parser_utils.hpp>

namespace margot {
namespace heel {

void parse(parameter& parameter, const boost::property_tree::ptree& parameter_node) {
  parameter.type = parameter_types::IMMEDIATE;
  parse(parameter.content, parameter_node);
  if (parameter.content.empty()) {
    // it's not an immediate, but we need to explore it further to fetch the var name and type
    // NOTE: it should be one value, but in case the user have done some weird stuff, we
    //       want to take only the last element
    for (const auto& pair : parameter_node) {
      parameter.content = pair.first;
      parse(parameter.value_type, pair.second);
    }
    parameter.type = parameter_types::VARIABLE;
  }
}

void parse(pair_property& property, const boost::property_tree::ptree& property_node) {
  for (const auto& pair : property_node) {
    property.key = pair.first;
    parse(property.value, pair.second);
  }
}

}  // namespace heel
}  // namespace margot