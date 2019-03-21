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


#include "beholder/parameters_beholder.hpp"

namespace beholder
{
  std::string Parameters_beholder::workspace_folder = "";
  bool Parameters_beholder::output_files = false;
  bool Parameters_beholder::use_clt = false;
  bool Parameters_beholder::use_difference_means_threshold = false;
  float Parameters_beholder::means_threshold_multiplier = 1.5;
  bool Parameters_beholder::disable_cohen_d_effect_size_check = false;
  float Parameters_beholder::cohen_d_threshold = 0.8;
  bool Parameters_beholder::disable_bonferroni_correction = false;
  int Parameters_beholder::window_size = 20;
  int Parameters_beholder::training_windows = 5;
  float Parameters_beholder::gamma_mean = 3;
  float Parameters_beholder::gamma_variance = 3;
  int Parameters_beholder::bad_clients_threshold = 20;
  bool Parameters_beholder::variance_off = false;
  int Parameters_beholder::min_observations = 20;
  int Parameters_beholder::timeout = 130;
  int Parameters_beholder::frequency_check = 30;
  float Parameters_beholder::alpha = 0.05;
  bool Parameters_beholder::no_trace_drop = false;
}
