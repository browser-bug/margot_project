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

#ifndef HEEL_CPP_UTILS_HDR
#define HEEL_CPP_UTILS_HDR

#include <cstdint>
#include <string>

#include <heel/model_state.hpp>

namespace margot {
namespace heel {

// this helper function generate the cpp code that translate a field "name" to its index
inline std::string generate_field_getter(const subject_kind kind, const std::string& field_name,
                                         const std::string& block_name) {
  const std::string section_name = kind == subject_kind::METRIC ? std::string("metric") : std::string("knob");
  return "static_cast<size_t>(margot::" + block_name + "::" + section_name + "::" + field_name + ")";
}

// this function generates the goal cpp identifier that must be used in the generated code
inline std::string generate_goal_identifier(const std::string& state_name, const std::size_t id) {
  return state_name + "_constraint_" + std::to_string(id);
}

// this function returns the name of the variable which holds the name of the file log
inline std::string generate_log_file_name_identifier(void) { return "margot_log_file_name_prefix"; }

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CPP_UTILS_HDR