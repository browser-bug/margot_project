/* beholder/ici_cdt.cc
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

#include "beholder/ici_cdt.hpp"
#include "beholder/parameters_beholder.hpp"
#include "agora/logger.hpp"

namespace beholder
{
  bool IciCdt::perform_ici_cdt(Data_ici_test& data_test, const std::vector<std::pair <float, std::string>>& window_pair)
  {
    // bool to stop the cycle when a change is detected
    bool change_detected_mean = false;
    bool change_detected_variance = false;

    // bool used to know whether the computed CI for variance is valid or if it is a NAN
    bool valid_variance = true;

    // increase the current windows number
    data_test.window_number++;

    // understand whether we are in the training phase or not
    if (data_test.window_number <= Parameters_beholder::training_windows)
    {
      // we are in training

      agora::pedantic("Training window number ", data_test.window_number, " out of ", Parameters_beholder::training_windows);

      // enqueue the current observation in the training set
      for (auto i : window_pair)
      {
        data_test.training_observations.emplace_back(i.first);
      }

      // compute the sample mean and the sample variance for the current window
      // sample mean M(s):
      float sum = 0;

      for (auto i : window_pair)
      {
        sum += i.first;
      }

      float mean = sum / Parameters_beholder::window_size;

      agora::pedantic("Window mean M(s=",  data_test.window_number, "): ", mean);

      // enqueue in the vector the sample mean M(s)
      data_test.training_sample_mean.emplace_back(mean);

      // sample variance S(s):
      if (!Parameters_beholder::variance_off)
      {
        sum = 0;

        for (auto i : window_pair)
        {
          sum += powf((i.first - mean), 2);
        }

        data_test.training_sample_variance.emplace_back(sum);
        agora::pedantic("Window variance S(s=",  data_test.window_number, "): ", sum);
      }


      if (data_test.window_number == Parameters_beholder::training_windows)
      {
        // this is the last training window
        agora::pedantic("RAINING FINISHED, computing test configuration parameters!");

        // compute the reference sample-mean mean
        sum = 0;

        for (auto i : data_test.training_sample_mean)
        {
          sum += i;
        }

        data_test.reference_sample_mean_mean = sum / Parameters_beholder::training_windows;
        // needed to have sample_mean_mean(s-1) for the else case (production phase)
        // case in which mu(S0) is also mu(s-1) for the next case
        data_test.current_sample_mean_mean = data_test.reference_sample_mean_mean;

        agora::pedantic("Reference_sample_mean_mean: ", data_test.reference_sample_mean_mean);

        // reference sample-mean variance
        sum = 0;

        for (auto i : data_test.training_sample_mean)
        {
          sum += powf((i - data_test.reference_sample_mean_mean), 2);
        }

        data_test.reference_sample_mean_variance = sqrtf(sum / (Parameters_beholder::training_windows - 1));
        agora::pedantic("Reference_sample_mean_variance: ", data_test.reference_sample_mean_variance);

        // computation of confidence interval for sample mean
        data_test.current_mean_conf_interval_lower = data_test.reference_sample_mean_mean - (Parameters_beholder::gamma_mean * data_test.reference_sample_mean_variance);
        data_test.current_mean_conf_interval_upper = data_test.reference_sample_mean_mean + (Parameters_beholder::gamma_mean * data_test.reference_sample_mean_variance);

        agora::pedantic("Training phase confidence interval for mean: [", data_test.current_mean_conf_interval_lower, ",", data_test.current_mean_conf_interval_upper, "]");

        // compute the reference sample-variance
        if (!Parameters_beholder::variance_off)
        {
          // compute the Gaussianizing Transform according to [Mudholkar81]
          // compute the first 6 raw Moments
          // https://en.wikipedia.org/wiki/Moment_(mathematics)
          std::vector<float> moments;

          for (int order = 1; order <= 6; order++)
          {
            sum = 0;

            for (auto i : data_test.training_observations)
            {
              // i-th observation raised to the power of "order"
              sum += powf(i, order);
            }

            // compute the mean
            moments.emplace_back(sum / data_test.training_observations.size());
            agora::pedantic("RawMoment[", order, "] = ", moments[order]);
          }

          // compute the cumulants of the original distribution
          // https://en.wikipedia.org/wiki/Cumulant#Cumulants_and_moments
          float c1 = moments[1];
          float c2 = moments[2] - powf(moments[1], 2);
          float c3 = 2 * powf(moments[1], 3) - 3 * moments[1] * moments[2] + moments[3];
          float c4 = -6 * powf(moments[1], 4) + 12 * powf(moments[1], 2) * moments[2] - 3 * powf(moments[2], 2) - 4 * moments[1] * moments[3] + moments[4];
          float c5 = 24 * powf(moments[1], 5) - 60 * powf(moments[1], 3) * moments[2] + 20 * powf(moments[1], 2) * moments[3] - 10 * moments[2] * moments[3] + 5 * moments[1] * (6 * powf(moments[2],
                     2) - moments[4]) + moments[5];
          float c6 = -120 * powf(moments[1], 6) + 360 * powf(moments[1], 4) * moments[2] - 270 * powf(moments[1], 2) * powf(moments[2], 2) + 30 * powf(moments[2], 3) - 120 * powf(moments[1],
                     3) * moments[3] + 120 * moments[1] * moments[2] * moments[3];
          c6 = c6 - 10 * powf(moments[3], 2) + 30 * (moments[1], 2) * moments[4] - 15 * moments[2] * moments[4] - 6 * moments[1] * moments[5] + moments[6];
          agora::pedantic("Cumulant[1] = ", c1);
          agora::pedantic("Cumulant[2] = ", c2);
          agora::pedantic("Cumulant[3] = ", c3);
          agora::pedantic("Cumulant[4] = ", c4);
          agora::pedantic("Cumulant[5] = ", c5);
          agora::pedantic("Cumulant[6] = ", c6);

          // compute sample variance moments
          float k1 = data_test.training_observations.size() - 1;
          float k2 = powf((data_test.training_observations.size() - 1), 2) * (c4 / (data_test.training_observations.size() * powf(c2, 2)) + 2 / (data_test.training_observations.size() - 1));
          float k3 = powf((data_test.training_observations.size() - 1), 3) * (c6 / powf(data_test.training_observations.size(),
                     2) + (12 * c4 * c2) / (data_test.training_observations.size() * (data_test.training_observations.size() - 1)) + (4 * (data_test.training_observations.size() - 2) * powf(c3,
                         2)) / (data_test.training_observations.size() * powf((data_test.training_observations.size() - 1), 2)) + (8 * powf(c2, 3)) / powf((data_test.training_observations.size() - 1), 2)) / powf(c2, 3);
          agora::pedantic("VarianceMoment[1] = ", k1);
          agora::pedantic("VarianceMoment[2] = ", k2);
          agora::pedantic("VarianceMoment[3] = ", k3);

          // compute h0
          data_test.h0 = 1 - ((k1 * k3) / (3 * powf(k2,  2)));
          agora::pedantic("h0 = ", data_test.h0);

          // compute V(s)=T(S(s)), the power-law transform gaussianization of the sample variance
          // use directly the sample variance w/o gaussianization
          for (auto i : data_test.training_sample_variance)
          {
            agora::pedantic("Sample Variance = ", i);
            data_test.training_sample_variance_transformed.emplace_back(pow((i / (Parameters_beholder::window_size - 1)), data_test.h0));
            agora::pedantic("Sample Variance Transformed = ", pow((i / (Parameters_beholder::window_size - 1)), data_test.h0));
          }

          // use the sample variance w/ gaussianization V(s)
          sum = 0;

          for (auto i : data_test.training_sample_variance_transformed)
          {
            sum += i;
          }

          data_test.reference_sample_variance_mean = sum / Parameters_beholder::training_windows;
          // needed to have sample_variance_mean(s-1) for the else case (production phase)
          // case in which mu(S0) is also mu(s-1) for the next case
          data_test.current_sample_variance_mean = data_test.reference_sample_variance_mean;

          // reference sample-variance variance
          sum = 0;

          for (auto i : data_test.training_sample_variance_transformed)
          {
            sum += powf((i - data_test.reference_sample_variance_mean), 2);
          }

          data_test.reference_sample_variance_variance = sqrtf(sum / (Parameters_beholder::training_windows - 1));
          // computation of confidence interval for sample variance
          data_test.current_variance_conf_interval_lower = data_test.reference_sample_variance_mean - (Parameters_beholder::gamma_variance * data_test.reference_sample_variance_variance);
          data_test.current_variance_conf_interval_upper = data_test.reference_sample_variance_mean + (Parameters_beholder::gamma_variance * data_test.reference_sample_variance_variance);

          agora::pedantic("Training phase confidence interval for variance: [", data_test.current_variance_conf_interval_lower, ",", data_test.current_variance_conf_interval_upper, "]");

          // check if the CI for variance is valid (i.e. if it is not a NAN)
          if (isnanf(data_test.current_variance_conf_interval_lower) || isnanf(data_test.current_variance_conf_interval_upper))
          {
            valid_variance = false;
          }


        }
      }
    }
    else
    {
      // we are in production phase
    }

    // return true if a change has been detected either in the mean or in the variance
    return change_detected_mean || change_detected_variance;
  }

}
