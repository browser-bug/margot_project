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

#include <heel/composer_metric.hpp>
#include <heel/composer_parameter.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_metric.hpp>
#include <heel/parser_tags.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& metric_node, const metric_model& metric) {
  // write the basic information about the metric
  metric_node.put(tag::name(), metric.name);
  metric_node.put(tag::type(), metric.type);
  metric_node.put(tag::distribution(), metric.distribution);
  metric_node.put(tag::prediction_plugin(), metric.prediction_plugin);
  metric_node.put(tag::observed_by(), metric.monitor_name);
  metric_node.put(tag::reactive_inertia(), metric.inertia);

  // write the values filed (we should have already computed any range tag)
  add_list(metric_node, metric.prediction_parameters, tag::prediction_params());
}

}  // namespace heel
}  // namespace margot