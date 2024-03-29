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

#ifndef HEEL_MODEL_MONITOR_HDR
#define HEEL_MODEL_MONITOR_HDR

#include <array>
#include <string>
#include <vector>

#include <heel/model_parameter.hpp>

namespace margot {
namespace heel {

// this struct holds information about the monitor types and usage
struct monitor_spec {
  std::string class_name;
  std::string header_name;
  std::string value_type;
  std::string start_method_name;
  std::string stop_method_name;
  std::vector<struct parameter> default_param_initialization;
  std::vector<struct parameter> default_param_start;
  std::vector<struct parameter> default_param_stop;
};

// this is the actual monitor model, with all the parameters
struct monitor_model {
  std::string name;
  std::string type;

  // NOTE: by convention the name of the statistic must be the same of the related method in the margot
  // statistical provider class
  std::vector<std::string> requested_statistics;  // e.g. average, standard_deviation, ...

  // these are the input parameters required to construct a monitor and to start/stop a measure
  std::vector<struct parameter> initialization_parameters;
  std::vector<struct parameter> start_parameters;
  std::vector<struct parameter> stop_parameters;
};

// this function validates a monitor model; i.e. it checks if there are missing or wrong information
void validate(monitor_model& model);

// these functions retrieves a copy of the spec of a known monitor
const monitor_spec& get_monitor_cpp_spec(monitor_model& monitor);
const monitor_spec& get_monitor_cpp_spec(const std::string& monitor_type);

// this function tells if this is a custom monitor
bool is_custom_monitor(const monitor_model& monitor);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_MONITOR_HDR