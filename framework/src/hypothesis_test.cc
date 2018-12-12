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

#include <cmath>

#include "beholder/hypothesis_test.hpp"
#include "beholder/parameters_beholder.hpp"
#include "agora/logger.hpp"

namespace beholder
{
  bool HypTest::perform_hypothesis_test(const std::unordered_map<std::string, std::pair < std::vector<float>, std::vector<float>>>& client_residuals_map)
  {
    // Prefix to log strings containing the app name and the metric name
    //std::string log_prefix = data_test.app_name + ":" + data_test.metric_name + "---";

    bool confirmed_change = false;

    // I'll have to go through each metric present in the structure, but as soon as i find a metric
    // which confirms the test i'll return true without analyzing the others

    // cycle over the metrics available
    for (auto i : client_residuals_map)
    {
      float t_statistic;
      float v_degree_freedom;

      // populations size
      int n1 = i.second.first.size();
      int n2 = i.second.second.size();

      // first population sample mean
      float x1 = 0;

      for (auto j : i.second.first)
      {
        x1 += j;
      }

      x1 = x1 / n1;
      // second population sample mean
      float x2 = 0;

      for (auto j : i.second.second)
      {
        x2 += j;
      }

      x2 = x2 / n2;

      // first population sample variance
      float s1_2 = 0;

      for (auto j : i.second.first)
      {
        s1_2 += powf(j - x1, 2);
      }

      s1_2 = s1_2 / (n1 - 1);
      // second population sample variance
      float s2_2 = 0;

      for (auto j : i.second.second)
      {
        s2_2 += powf(j - x2, 2);
      }

      s2_2 = s2_2 / (n2 - 1);

      float temp = (s1_2 / n1) + (s2_2 / n2);

      // t statistic computation
      t_statistic = (x1 - x2) / (sqrtf(temp));

      // degree of freedom associated with the variance estimates
      float v1 = n1 - 1;
      float v2 = n2 - 1;

      // v degree of freedom computation with the Welch–Satterthwaite equation
      v_degree_freedom = powf(temp, 2) / ((powf(s1_2, 2) / (powf(n1, 2) * v1)) + (powf(s2_2, 2) / (powf(n2, 2) * v2)));



      // at the first metric which confirms the change return to the caller
      if (confirmed_change)
      {
        return confirmed_change;
      }
    }

    return confirmed_change;
  }
}
