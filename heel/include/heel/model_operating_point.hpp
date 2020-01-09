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

#ifndef HEEL_MODEL_OPERATING_POINT_HDR
#define HEEL_MODEL_OPERATING_POINT_HDR

#include <string>
#include <vector>

namespace margot {
namespace heel {

struct operating_point_value {
  std::string mean;
  std::string standard_deviation;
};

struct operating_point_model {
  std::vector<operating_point_value> features;
  std::vector<operating_point_value> knobs;
  std::vector<operating_point_value> metrics;
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_OPERATING_POINT_HDR