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

#include <heel/composer_agora.hpp>
#include <heel/composer_parameter.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_agora.hpp>
#include <heel/parser_tags.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& agora_node, const agora_model& agora) {
  // write the base information to operate with agora
  agora_node.put(tag::url(), agora.url);
  agora_node.put(tag::username(), agora.username);
  agora_node.put(tag::password(), agora.password);
  agora_node.put(tag::qos(), agora.qos);
  agora_node.put(tag::broker_ca(), agora.broker_ca);
  agora_node.put(tag::client_cert(), agora.client_cert);
  agora_node.put(tag::client_key(), agora.client_key);
  agora_node.put(tag::number_configurations_per_iteration(), agora.number_configurations_per_iteration);
  agora_node.put(tag::number_observations_per_configuration(), agora.number_observations_per_configuration);
  agora_node.put(tag::max_number_iteration(), agora.max_number_iteration);
  agora_node.put(tag::doe_plugin(), agora.doe_plugin);
  agora_node.put(tag::clustering_plugin(), agora.clustering_plugin);

  // write the optional parameter for the
  add_list(agora_node, agora.doe_parameters, tag::doe_parameters());
  add_list(agora_node, agora.clustering_parameters, tag::clustering_parameters());
}

}  // namespace heel
}  // namespace margot
