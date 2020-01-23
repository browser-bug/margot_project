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

#ifndef HEEL_MODEL_BLOCK_HDR
#define HEEL_MODEL_BLOCK_HDR

#include <string>
#include <vector>

#include <heel/model_agora.hpp>
#include <heel/model_features.hpp>
#include <heel/model_knob.hpp>
#include <heel/model_metric.hpp>
#include <heel/model_monitor.hpp>
#include <heel/model_operating_point.hpp>
#include <heel/model_state.hpp>

namespace margot {
namespace heel {

struct block_model {
  std::string name;
  std::vector<monitor_model> monitors;
  std::vector<knob_model> knobs;
  std::string knobs_segment_type;
  std::vector<metric_model> metrics;
  std::string metrics_segment_type;
  features_model features;
  agora_model agora;
  std::vector<state_model> states;
  std::vector<operating_point_model> ops;
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_BLOCK_HDR