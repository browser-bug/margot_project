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
#include <heel/model_agora.hpp>

namespace margot {
namespace heel {

void validate(agora_model& model, const std::vector<metric_model>& metrics,
              const std::vector<knob_model>& knobs) {
  // the validation of this model is useful only if agora is enabled in the configuration file
  if (!model.empty()) {
    // at first check if the user provided at least a broker url and a qos
    if (model.url.empty()) {
      error("To use Agora it is required the url of the MQTT broker");
      throw std::runtime_error("agora model: empty broker url");
    }
    if (model.qos.empty()) {
      error("To use Agora it is required the set the communication level of QoS");
      throw std::runtime_error("agora model: empty connection qos");
    }
    if ((model.qos.compare("0") != 0) && (model.qos.compare("1") != 0) && (model.qos.compare("2") != 0)) {
      error("Unknown MQTT QoS level \"", model.qos, "\", should be an integer number in interval (0,2)");
      throw std::runtime_error("agora model: unknown qos level");
    }

    // then, we need to check if the user set the name of the required plugin
    if (model.doe_plugin.empty()) {
      error("To use Agora it is required the set the name of the plugin that performs the doe");
      throw std::runtime_error("agora model: empty doe plugin");
    }
    if (model.clustering_plugin.empty()) {
      error("To use Agora it is required the set the name of the plugin that performs the clustering");
      throw std::runtime_error("agora model: empty clustering plugin");
    }

    // moreover, there should be at least one metric to model it with agora at runtime
    if (metrics.empty()) {
      error("Using Agora is meaningful if there is at least one metric");
      throw std::runtime_error("agora error: no metric to predict");
    }

    // then, we need to make sure that all the metrics are observed at runtime
    if (std::any_of(metrics.begin(), metrics.end(),
                    [](const metric_model& metric) { return metric.monitor_name.empty(); })) {
      error(
          "At least one metric is not observed by a monitor at runtime, unable to use Agora to derive the "
          "application knowledge");
      throw std::runtime_error("agora error: metric not observed");
    }

    // then, we need to ensure that all the metrics specify a plugin to generate the application knoledge
    if (std::any_of(metrics.begin(), metrics.end(),
                    [](const metric_model& metric) { return metric.prediction_plugin.empty(); })) {
      error("At least one metric has no plugin to predict its performance");
      throw std::runtime_error("agora error: metric not predicted");
    }

    // finally, we need to ensure that all the knobs have a non-empty range of values
    if (std::any_of(knobs.begin(), knobs.end(), [](const knob_model& knob) { return knob.values.empty(); })) {
      error("At least one knob has no range of values, it is impossible to explore");
      throw std::runtime_error("agora error: unknown knob range");
    }
  }
}

}  // namespace heel
}  // namespace margot