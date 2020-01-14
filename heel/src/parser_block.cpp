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
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/logger.hpp>
#include <heel/parser_agora.hpp>
#include <heel/parser_block.hpp>
#include <heel/parser_features.hpp>
#include <heel/parser_knob.hpp>
#include <heel/parser_metric.hpp>
#include <heel/parser_monitor.hpp>
#include <heel/parser_state.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

namespace margot {
namespace heel {

void parse(block_model& block, const boost::property_tree::ptree& block_node) {
  // parse the name, metrics and knobs of the block
  parse_element(block.name, block_node, tag::name());
  parse_list(block.monitors, block_node, tag::monitors());
  parse_list(block.knobs, block_node, tag::knobs());
  parse_list(block.metrics, block_node, tag::metrics());

  // sort them according to their name
  std::sort(block.monitors.begin(), block.monitors.end(),
            [](const monitor_model& i, const monitor_model& j) { return i.name < j.name; });
  std::sort(block.knobs.begin(), block.knobs.end(),
            [](const knob_model& i, const knob_model& j) { return i.name < j.name; });
  std::sort(block.metrics.begin(), block.metrics.end(),
            [](const metric_model& i, const metric_model& j) { return i.name < j.name; });

  // parsing the application features is more complicated, since it is an optional section and it
  // contains an enum, that we have to parse and understand
  std::string feature_distance;
  parse_element(feature_distance, block_node, tag::feature_distance());
  if (is_enum(feature_distance, features_distance_type::EUCLIDEAN)) {
    block.features.distance_type = features_distance_type::EUCLIDEAN;
  } else if (is_enum(feature_distance, features_distance_type::NORMALIZED)) {
    block.features.distance_type = features_distance_type::NORMALIZED;
  } else if (!feature_distance.empty()) {
    error("Unknown feature distance \"", feature_distance, "\" in block \"", block.name, "\"");
    throw std::runtime_error("block parser: unknown feature distance");
  } else {
    block.features.distance_type = features_distance_type::NONE;
  }
  parse_list(block.features.fields, block_node, tag::features());

  // sort the features list field
  std::sort(block.features.fields.begin(), block.features.fields.end(),
            [](const feature_model& i, const feature_model& j) { return i.name < j.name; });

  // now we can parse the remainder of the block fields
  parse_element(block.agora, block_node, tag::agora());
  parse_list(block.states, block_node, tag::states());

  // the operating points are parsed in a different time, since they are no really part of the model
}

}  // namespace heel
}  // namespace margot