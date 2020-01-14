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

#include <algorithm>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_application.hpp>
#include <heel/parser_application.hpp>
#include <heel/parser_block.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

namespace margot {
namespace heel {

// this function parses the few information about the application, then it parse the main part of the
// configuration file: the blocks of the application
void parse(application_model& application, const boost::property_tree::ptree& application_node) {
  parse_element(application.name, application_node, tag::name());
  parse_element(application.version, application_node, tag::version());
  parse_list(application.blocks, application_node, tag::blocks());
  std::sort(application.blocks.begin(), application.blocks.end(),
            [](const block_model& i, const block_model& j) { return i.name < j.name; });
}

}  // namespace heel
}  // namespace margot