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

#ifndef HEEL_MODEL_PARAMETER_HDR
#define HEEL_MODEL_PARAMETER_HDR

#include <string>

namespace margot {
namespace heel {

// this enum discriminates between different types of parameters for the
// representation of the extra-functional requirements. In particular, immediate
// values are C++ constants that are known at compile time (e.g. numbers), while
// variable values are C++ variables that are not known at compile time.
enum class parameter_types { IMMEDIATE, VARIABLE };

struct parameter {
  parameter_types type;
  std::string content;     // its semantic depends on the parameter type (e.g. a
                           // number or variable name)
  std::string value_type;  // e.g. int, long unsigned int, double, etc.
};

// this class represents a more simple parameter on the <key>:<value> form
struct pair_property {
  std::string key;
  std::string value;
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_PARAMETER_HDR