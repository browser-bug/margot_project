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

#ifndef HEEL_CPP_UPDATE_GEN_HDR
#define HEEL_CPP_UPDATE_GEN_HDR

#include <string>

#include <heel/generator_utils.hpp>
#include <heel/model_block.hpp>
#include <heel/model_features.hpp>
#include <heel/model_knob.hpp>

namespace margot {
namespace heel {

// this is an helper struct to write the declaration of input/output parameters
struct cpp_update_gen {
  static std::string signature(const block_model& block) {
    const std::string feature_params =
        join(block.features.fields.begin(), block.features.fields.end(), ", ",
             [](const feature_model& f) { return "const " + f.type + " " + f.name; });
    const std::string knob_params =
        join(block.knobs.begin(), block.knobs.end(), ", ", [](const knob_model& knob) {
          return knob.type.compare("string") != 0 ? knob.type + "& " + knob.name
                                                  : "std::string& " + knob.name;
        });
    return !feature_params.empty() ? feature_params + ", " + knob_params : knob_params;
  }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CPP_UPDATE_GEN_HDR