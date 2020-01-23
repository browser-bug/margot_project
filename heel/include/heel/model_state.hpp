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

#ifndef HEEL_MODEL_STATE_HDR
#define HEEL_MODEL_STATE_HDR

#include <cstdint>
#include <string>
#include <vector>

#include <heel/model_knob.hpp>
#include <heel/model_metric.hpp>

namespace margot {
namespace heel {

enum class goal_comparison { LESS_OR_EQUAL, GREATER_OR_EQUAL, LESS, GREATER };
enum class rank_direction { MINIMIZE, MAXIMIZE, NONE };
enum class rank_type { SIMPLE, GEOMETRIC, LINEAR, NONE };
enum class subject_kind { METRIC, KNOB, UNKNOWN };

struct constraint_model {
  std::string name;
  goal_comparison cfun;
  std::string value;
  subject_kind kind;
  std::string confidence;
};

struct rank_field_model {
  std::string name;
  subject_kind kind;
  std::string coefficient;
};

struct state_model {
  std::string name;
  rank_direction direction;
  rank_type combination;
  std::vector<rank_field_model> rank_fields;
  std::vector<constraint_model> constraints;
};

// this function validates a state model to enforce a correct configuration of the application
void validate(state_model& model, const std::vector<metric_model>& metrics,
              const std::vector<knob_model>& knobs);

// utility function that translates a comparison function to a string
inline std::string to_str(const margot::heel::goal_comparison combination) {
  switch (combination) {
    case margot::heel::goal_comparison::LESS_OR_EQUAL:
      return "le";
    case margot::heel::goal_comparison::GREATER_OR_EQUAL:
      return "ge";
    case margot::heel::goal_comparison::LESS:
      return "lt";
    case margot::heel::goal_comparison::GREATER:
      return "gt";
    default:
      return "defensive programming: unknown distance type";
  };
}
inline std::string to_str(const margot::heel::rank_direction direction) {
  switch (direction) {
    case margot::heel::rank_direction::MINIMIZE:
      return "minimize";
    case margot::heel::rank_direction::MAXIMIZE:
      return "maximize";
    case margot::heel::rank_direction::NONE:
      return "none";
    default:
      return "defensive programming: unknown direction type";
  };
}
inline std::string to_str(const margot::heel::rank_type type) {
  switch (type) {
    case margot::heel::rank_type::SIMPLE:
      return "simple_mean";
    case margot::heel::rank_type::GEOMETRIC:
      return "geometric_mean";
    case margot::heel::rank_type::LINEAR:
      return "linear_mean";
    case margot::heel::rank_type::NONE:
      return "none";
    default:
      return "defensive programming: unknown rank type";
  };
}

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_STATE_HDR