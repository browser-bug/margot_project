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
#include <stdexcept>
#include <vector>

#include <heel/logger.hpp>
#include <heel/model_metric.hpp>
#include <heel/model_monitor.hpp>
#include <heel/typer.hpp>

namespace margot {
namespace heel {

void validate(metric_model& model, const std::vector<monitor_model>& monitors) {
  // perform checks on the metric type
  if (model.type.empty()) {
    error("The metric \"", model.name, "\" must have a type");
    throw std::runtime_error("metric model: metric without a type");
  }
  model.type = sanitize_type(model.type);
  if (model.type.compare("string") == 0) {
    error("The type string for the metric \"", model.name, "\" is not supported");
    throw std::runtime_error("metric model: unsupported type");
  }

  // check if the name is a valid c/c++ identifier
  if (!is_valid_identifier(model.name)) {
    error("The metric name \"", model.name, "\" is not a valid c/c++ identifier");
    throw std::runtime_error("metric model: unsupported name");
  }

  // make sure that if plugin parameters exist, there should be also a plugin name
  if ((!model.prediction_parameters.empty()) && (model.prediction_plugin.empty())) {
    error("The metric \"", model.name, "\" defines plugin parameters, but no plugin");
    throw std::runtime_error("metric model: empty prediction plugin");
  }

  // now we need to be sure that if a metric is observed by a monitor, the monitor exists
  if (!model.monitor_name.empty()) {
    if (!std::any_of(monitors.cbegin(), monitors.cend(), [&model](const monitor_model& monitor) {
          return model.monitor_name.compare(monitor.name) == 0;
        })) {
      error("The metric \"", model.name, "\" is observed by the non-existent monitor \"", model.monitor_name,
            "\"");
      throw std::runtime_error("metric model: non-existent monitor");
    }
  }

  // finally, we need to be sure that if we react with runtime observation, it must be observed by a monitor
  if (model.inertia > 0 && model.monitor_name.empty()) {
    error("The metric \"", model.name, "\" can't react without a monitor\"");
    throw std::runtime_error("metric model: unable to active the reaction mechanism");
  }
}

}  // namespace heel
}  // namespace margot