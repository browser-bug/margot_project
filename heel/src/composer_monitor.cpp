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
#include <utility>

#include <boost/property_tree/ptree.hpp>

#include <heel/composer_monitor.hpp>
#include <heel/composer_parameter.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_monitor.hpp>
#include <heel/parser_tags.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& monitor_node, const monitor_model& monitor) {
  // write the basic information about the monitor
  monitor_node.put(tag::name(), monitor.name);
  monitor_node.put(tag::type(), monitor.type);

  // write all the other information
  add_list(monitor_node, monitor.requested_statistics, tag::log());
  add_list(monitor_node, monitor.initialization_parameters, tag::constructor());
  add_list(monitor_node, monitor.start_parameters, tag::start());
  add_list(monitor_node, monitor.stop_parameters, tag::stop());
}

}  // namespace heel
}  // namespace margot