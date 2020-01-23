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
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

#include <heel/logger.hpp>
#include <heel/model_metric.hpp>
#include <heel/parser_metric.hpp>
#include <heel/parser_parameter.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace margot {
namespace heel {

void parse(metric_model& metric, const boost::property_tree::ptree& metric_node) {
  // parse the easiest information of the metric
  parse_element(metric.name, metric_node, tag::name());
  parse_element(metric.type, metric_node, tag::type());
  parse_element(metric.monitor_name, metric_node, tag::observed_by());
  parse_element(metric.prediction_plugin, metric_node, tag::prediction_plugin());
  parse_list(metric.prediction_parameters, metric_node, tag::prediction_params());

  // now we need to parse the inertia
  std::string inertia_str;
  parse_element(inertia_str, metric_node, tag::reactive_inertia());
  metric.inertia = !inertia_str.empty() ? boost::lexical_cast<std::size_t>(inertia_str) : 0;

  // new we have to parse the distribution tag
  std::string distribution_str;
  parse_element(distribution_str, metric_node, tag::distribution());
  if (distribution_str.empty()) {
    metric.distribution = false;
    info("Automatically set metric \"", metric.name, "\" distribution default value \"", metric.distribution,
         "\"");
  } else {
    const bool is_yes = !distribution_str.empty() ? is_bool(distribution_str, true) : false;
    const bool is_no = !distribution_str.empty() ? is_bool(distribution_str, false) : false;
    if (!is_yes && !is_no) {
      error("Unable to understand the distribution value \"", distribution_str, "\" in metric \"",
            metric.name, "\"");
      throw std::runtime_error("Unknown distribution value");
    } else if (is_yes) {
      metric.distribution = true;
    } else if (is_no) {
      metric.distribution = false;
    }
  }
}

}  // namespace heel
}  // namespace margot
