/* beholder/common_objects_beholder.hpp
 * Copyright (C) 2018 Alberto Bendin
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


#ifndef MARGOT_BEHOLDER_COMMON_OBJECTS_BEHOLDER
#define MARGOT_BEHOLDER_COMMON_OBJECTS_BEHOLDER

#include <fstream>

namespace beholder
{

  // data structure for the files to be exported ready to be used with the provided gnuplot script
  struct output_files
  {
    std::fstream observations;
    std::fstream ici;
  };

  // struct to bind a residual with its timestamp
  struct residual_timestamp_struct
  {
    float residual_value;
    std::string residual_timestamp;
  };

  // struct to store the observations from trace respectively in the before/after_change
  // vector according to their position wrt the hypothetical change window
  struct residuals_from_trace
  {
    std::vector<float> before_change;
    std::vector<float> after_change;
  };

}

#endif // MARGOT_BEHOLDER_COMMON_OBJECTS_BEHOLDER
