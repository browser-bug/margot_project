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

#include <heel/composer_application.hpp>
#include <heel/composer_block.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_application.hpp>
#include <heel/parser_tags.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& root_node, const application_model& app) {
  // insert the basic information about the application
  root_node.put(tag::name(), app.name);
  root_node.put(tag::version(), app.name);

  // add the list of blocks
  add_list(root_node, app.blocks, tag::blocks());

  // add the application name for each block
  for(auto& block : app.blocks) {
    block.application_name = app.name;
  }
}

}  // namespace heel
}  // namespace margot