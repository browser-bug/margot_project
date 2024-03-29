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

#ifndef HEEL_PRINT_APPLICATION_MODEL_HDR
#define HEEL_PRINT_APPLICATION_MODEL_HDR

#include <ostream>
#include <sstream>
#include <string>

#include <heel/generator_description_verbose.hpp>
#include <heel/model_application.hpp>

namespace margot {
namespace heel {

template <class CharT, class Traits = std::char_traits<CharT> >
void print_application_model(const application_model& model, std::basic_ostream<CharT, Traits>& out) {
  // print the general information about the application model
  out << "//========================================================================//" << std::endl;
  out << "// Model of \"" << model.name << "\" version \"" << model.version << "\"" << std::endl;
  out << "//========================================================================//" << std::endl;

  // define a lambda to format the component descriptions
  const auto formatter = [&out](std::stringstream&& description) {
    const std::string prefix = "// ";
    std::string description_line;
    out << prefix << std::endl;
    while (std::getline(description, description_line)) {
      out << prefix << description_line << std::endl;
    }
  };

  // now we should loop over the blocks that constitute the application
  for (const auto& block : model.blocks) {
    out << "//" << std::endl;
    out << "//---------------------------------------- Block \"" << block.name << "\"" << std::endl;
    for (const auto& monitor : block.monitors) {
      formatter(margot::heel::description_verbose(monitor));
    }
    for (const auto& knob : block.knobs) {
      formatter(margot::heel::description_verbose(knob));
    }
    for (const auto& metric : block.metrics) {
      formatter(margot::heel::description_verbose(metric));
    }
    formatter(margot::heel::description_verbose(block.features));
    if (!block.agora.empty()) {
      formatter(margot::heel::description_verbose(block.agora));
    }
    for (const auto& state : block.states) {
      formatter(margot::heel::description_verbose(state));
    }
  }
  out << "//" << std::endl;
  out << "//========================================================================//" << std::endl;
}

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PRINT_APPLICATION_MODEL_HDR