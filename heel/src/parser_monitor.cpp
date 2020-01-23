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

#include <algorithm>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_monitor.hpp>
#include <heel/parser_monitor.hpp>
#include <heel/parser_parameter.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace margot {
namespace heel {

void parse(monitor_model& monitor, const boost::property_tree::ptree& monitor_node) {
  parse_element(monitor.name, monitor_node, tag::name());
  parse_element(monitor.type, monitor_node, tag::type());
  parse_list(monitor.requested_statistics, monitor_node, tag::log());
  parse_list(monitor.initialization_parameters, monitor_node, tag::constructor());
  parse_list(monitor.start_parameters, monitor_node, tag::start());
  parse_list(monitor.stop_parameters, monitor_node, tag::stop());
}

}  // namespace heel
}  // namespace margot