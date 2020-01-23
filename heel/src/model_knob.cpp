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

#include <stdexcept>
#include <vector>

#include <heel/logger.hpp>
#include <heel/model_knob.hpp>
#include <heel/typer.hpp>

namespace margot {
namespace heel {

void validate(knob_model& model) {
  // check if the knob has a type
  if (model.type.empty()) {
    error("The knob \"", model.name, "\" must have a type");
    throw std::runtime_error("knob model: knob without a type");
  }

  // sanitize the knob type
  model.type = sanitize_type(model.type);

  // check if the name is a valid c/c++ identifier
  if (!is_valid_identifier(model.name)) {
    error("The knob name \"", model.name, "\" is not a valid c/c++ identifier");
    throw std::runtime_error("knob model: unsupported name");
  }

  // check if the type is a string, we must also have range of values, since we can use them to construct
  // translator to map a string to an integer
  if ((model.type.compare("string") == 0) && (model.values.empty())) {
    error("The type string for the knob \"", model.name, "\" also requires the list of possible values");
    throw std::runtime_error("knob model: string enum missing");
  }
}

}  // namespace heel
}  // namespace margot
