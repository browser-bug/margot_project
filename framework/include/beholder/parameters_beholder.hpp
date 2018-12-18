/* beholder/parameters_beholder.hpp
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


#ifndef MARGOT_BEHOLDER_BEHOLDER_PARAMETERS
#define MARGOT_BEHOLDER_BEHOLDER_PARAMETERS

#include <string>

namespace beholder
{

  struct Parameters_beholder
  {
    static std::string workspace_folder;
    static bool output_files;
    static int window_size;
    static int training_windows;
    static float gamma_mean;
    static float gamma_variance;
    static int bad_clients_threshold;
    static bool variance_off;
    static int min_observations;
    static int timeout;
    static int frequency_check;
    static float alpha;
    static bool no_trace_drop;
  };

}

#endif // MARGOT_BEHOLDER_BEHOLDER_PARAMETERS
