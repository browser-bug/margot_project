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

#include <string>

#include <heel/model_agora.hpp>
#include <heel/parser_agora.hpp>
#include <heel/parser_parameter.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace margot {
namespace heel {

// this function basically fetch all the properties inside an agora element, with special care for the doe
// properties, that can vary according to the selected doe plugin
void parse(agora_model& agora, const boost::property_tree::ptree& agora_node) {
  parse_element(agora.url, agora_node, tag::url());
  parse_element(agora.username, agora_node, tag::username());
  parse_element(agora.password, agora_node, tag::password());
  parse_element(agora.qos, agora_node, tag::qos());
  parse_element(agora.broker_ca, agora_node, tag::broker_ca());
  parse_element(agora.client_cert, agora_node, tag::client_cert());
  parse_element(agora.client_key, agora_node, tag::client_key());
  parse_element(agora.doe_plugin, agora_node, tag::doe_plugin());
  parse_element(agora.clustering_plugin, agora_node, tag::clustering_plugin());
  parse_element(agora.url, agora_node, tag::url());
  parse_element(agora.url, agora_node, tag::url());
  parse_list(agora.doe_parameters, agora_node, tag::doe_parameters());
  parse_list(agora.clustering_parameters, agora_node, tag::clustering_parameters());
}

}  // namespace heel
}  // namespace margot