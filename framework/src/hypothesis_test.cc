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
#include <boost/math/distributions/students_t.hpp>
using boost::math::students_t;

#include "beholder/hypothesis_test.hpp" //TODO: do we really need this??
#include "beholder/parameters_beholder.hpp"
#include "agora/logger.hpp"

namespace beholder
{
  bool HypTest::perform_hypothesis_test(const std::unordered_map<std::string, std::pair < std::vector<float>, std::vector<float>>>& client_residuals_map, const std::string& application_name,
                                        const std::string& client_name)
  {

    bool confirmed_change = false;

    // I'll have to go through each metric present in the structure, but as soon as i find a metric
    // which confirms the test i'll return true without analyzing the others

    // cycle over the metrics available
    for (auto i : client_residuals_map)
    {
      // Prefix to log strings containing the app name, the client name and the metric name
      std::string log_prefix = "HYP_TEST:" + application_name + ":" + client_name + ":" + i.first + "---";

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

      //
      // Define our distribution, and get the probability:
      // https://www.boost.org/doc/libs/1_69_0/libs/math/doc/html/math_toolkit/stat_tut/weg/st_eg/two_sample_students_t.html
      // https://www.boost.org/doc/libs/1_69_0/libs/math/example/students_t_two_samples.cpp
      //
      students_t dist(v_degree_freedom);

      // find the critical value
      // the one usually found on the table, with the difference that here we can avoid rounding
      // the v (degree of freedom) to the nearest integer.
      float q = cdf(complement(dist, fabs(t_statistic)));
      // Here we've used the absolute value of the t-statistic, because we initially want to know
      // simply whether there is a difference or not (a two-sided test).
      // The Null-hypothesis: there is no difference in means. Reject if complement of CDF for |t| < significance level / 2:
      // The Alternative-hypothesis: there is a difference in means. Reject if complement of CDF for |t| > significance level / 2:
      // In our situation the change is confirmed when the null hypothesis (no change) is rejected,
      // and then the alternative hypothesis is not rejected.

      if (q < Parameters_beholder::alpha / 2)
      {
        confirmed_change = true;
        agora::info(log_prefix, "HYPOTHESIS TEST, change confirmed!");
      }
      else
      {
        agora::info(log_prefix, "HYPOTHESIS TEST, change rejected!");
      }

      // at the first metric which confirms the change return to the caller
      if (confirmed_change)
      {
        return confirmed_change;
      }
    }

    return confirmed_change;
  }
}
