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

  int Parameters_beholder::window_size = 20;
  int Parameters_beholder::training_windows = 5;
  float Parameters_beholder::gamma_mean = 1;
  float Parameters_beholder::gamma_variance = 1;
  int Parameters_beholder::bad_clients_threshold = 20;

}
