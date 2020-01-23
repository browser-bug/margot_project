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

#ifndef HEEL_CPP_INIT_GEN_HDR
#define HEEL_CPP_INIT_GEN_HDR

#include <algorithm>
#include <string>

#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/model_monitor.hpp>
#include <heel/model_parameter.hpp>

namespace margot {
namespace heel {

// this is an helper struct to write the declaration of input/output parameters
struct cpp_init_gen {
  static std::string signature(const application_model& app) {
    std::string signature_line;
    std::for_each(app.blocks.begin(), app.blocks.end(), [&signature_line](const block_model& block) {
      std::for_each(block.monitors.begin(), block.monitors.end(),
                    [&signature_line, &block](const monitor_model& monitor) {
                      std::for_each(
                          monitor.initialization_parameters.begin(), monitor.initialization_parameters.end(),
                          [&signature_line, &block](const parameter& p) {
                            if (p.type == parameter_types::VARIABLE) {
                              signature_line += !signature_line.empty() ? std::string(", ") : std::string("");
                              signature_line += p.value_type + " " + block.name + "_" + p.content;
                            }
                          });  // end for each initialization parameters
                    });        // end for each monitor
    });                        // end for each block
    return signature_line;
  }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CPP_INIT_GEN_HDR