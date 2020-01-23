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

#ifndef HEEL_CPP_PARSER_GEN_HDR
#define HEEL_CPP_PARSER_GEN_HDR

#include <string>
#include <vector>

#include <heel/cpp_utils.hpp>
#include <heel/generator_utils.hpp>
#include <heel/model_features.hpp>
#include <heel/model_knob.hpp>
#include <heel/model_metric.hpp>

namespace margot {
namespace heel {

// this is an helper struct to write the declaration of input/output parameters
struct cpp_parser_gen {
  inline static std::string signature(const std::vector<feature_model>& fields,
                                      const std::vector<knob_model>& knobs,
                                      const std::vector<metric_model>& metrics) {
    std::string result;
    if (!fields.empty()) {
      result = margot::heel::join(fields.begin(), fields.end(), ", ", [](const feature_model& field) {
        return "const " + field.type + " " + field.name;
      });
      result += ", ";
    }
    result += margot::heel::join(knobs.begin(), knobs.end(), ", ", [](const knob_model& knob) {
      return knob.type.compare("string") == 0 ? std::string("const std::string& ") + knob.name
                                              : "const " + knob.type + " " + knob.name;
    });
    result += ", ";
    result += margot::heel::join(metrics.begin(), metrics.end(), ", ", [](const metric_model& metric) {
      return "const " + metric.type + " " + metric.name;
    });
    return result;
  }

  inline static std::string usage(const std::vector<feature_model>& fields,
                                  const std::vector<knob_model>& knobs,
                                  const std::vector<metric_model>& metrics) {
    std::string result;
    if (!fields.empty()) {
      result = margot::heel::join(fields.begin(), fields.end(), ", ",
                                  [](const feature_model& field) { return "c.features." + field.name; });
      result += ", ";
    }
    result += margot::heel::join(knobs.begin(), knobs.end(), ", ",
                                 [](const knob_model& knob) { return "c.knobs." + knob.name; });
    result += ", ";
    result += margot::heel::join(metrics.begin(), metrics.end(), ", ", [](const metric_model& metric) {
      return "c.monitors." + metric.monitor_name + ".last()";
    });
    return result;
  }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CPP_PARSER_GEN_HDR