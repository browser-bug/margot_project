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

#ifndef HEEL_MODEL_METRIC_HDR
#define HEEL_MODEL_METRIC_HDR

#include <cstdint>
#include <string>
#include <vector>

#include <heel/model_monitor.hpp>
#include <heel/model_parameter.hpp>

namespace margot {
namespace heel {

struct metric_model {
  std::string name;
  std::string type;
  bool distribution;
  std::string prediction_plugin;
  std::string monitor_name;
  std::size_t inertia;  // zero means no reaction
  std::vector<pair_property> prediction_parameters;
};

// this function validates a metric model to enforce a correct configuration of the application
void validate(metric_model& model, const std::vector<monitor_model>& monitors);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_METRIC_HDR