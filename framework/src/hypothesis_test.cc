/* beholder/hypothesis_test.cc
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


#include "beholder/hypothesis_test.hpp"
#include "beholder/parameters_beholder.hpp"
#include "agora/logger.hpp"

namespace beholder
{
  bool HypTest::perform_hypothesis_test(const std::unordered_map<std::string, std::pair < std::vector<float>, std::vector<float>>>& client_residuals_map)
  {
    // Prefix to log strings containing the app name and the metric name
    //std::string log_prefix = data_test.app_name + ":" + data_test.metric_name + "---";

    // I'll have to go through each metric present in the structure, but as soon as i find a metric
    // which confirms the test i'll return true without analyzing the others

    return false;
  }
}
