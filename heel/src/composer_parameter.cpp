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

#include <stdexcept>

#include <boost/property_tree/ptree.hpp>

#include <heel/composer_parameter.hpp>
#include <heel/model_parameter.hpp>
#include <heel/parser_tags.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& parameter_node, const parameter& parameter) {
  switch (parameter.type) {
    case parameter_types::IMMEDIATE:
      parameter_node.put("", parameter.content);
      break;
    case parameter_types::VARIABLE:
      parameter_node.put(parameter.content, parameter.value_type);
      break;
    default:
      throw std::runtime_error("Unable to compse a parameter with an unknown parameter type");
  }
}

void compose(boost::property_tree::ptree& property_node, const pair_property& property) {
  property_node.put(property.key, property.value);
}

}  // namespace heel
}  // namespace margot
