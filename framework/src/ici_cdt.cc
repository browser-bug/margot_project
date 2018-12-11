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

    // Prefix to log strings containing the app name and the metric name
    std::string log_prefix = data_test.app_name + ":" + data_test.metric_name + "---";

    // understand whether we are in the training phase or not
    if (data_test.window_number <= Parameters_beholder::training_windows)
    {
      // we are in training

      agora::pedantic(log_prefix, "Training window number ", data_test.window_number, " out of ", Parameters_beholder::training_windows);

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

      agora::pedantic(log_prefix, "Window mean M(s=",  data_test.window_number, "): ", mean);

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
        agora::pedantic(log_prefix, "Window variance S(s=",  data_test.window_number, "): ", sum);
      }


      if (data_test.window_number == Parameters_beholder::training_windows)
      {
        // this is the last training window
        agora::info(log_prefix, "TRAINING FINISHED, computing test configuration parameters!");

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

        agora::pedantic(log_prefix, "Reference_sample_mean_mean: ", data_test.reference_sample_mean_mean);

        // reference sample-mean variance
        sum = 0;

        for (auto i : data_test.training_sample_mean)
        {
          sum += powf((i - data_test.reference_sample_mean_mean), 2);
        }

        data_test.reference_sample_mean_variance = sqrtf(sum / (Parameters_beholder::training_windows - 1));
        agora::pedantic(log_prefix, "Reference_sample_mean_variance: ", data_test.reference_sample_mean_variance);

        // computation of confidence interval for sample mean
        data_test.current_mean_conf_interval_lower = data_test.reference_sample_mean_mean - (Parameters_beholder::gamma_mean * data_test.reference_sample_mean_variance);
        data_test.current_mean_conf_interval_upper = data_test.reference_sample_mean_mean + (Parameters_beholder::gamma_mean * data_test.reference_sample_mean_variance);

        agora::pedantic(log_prefix, "Training phase confidence interval for mean: [", data_test.current_mean_conf_interval_lower, ",", data_test.current_mean_conf_interval_upper, "]");

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
            agora::pedantic(log_prefix, "RawMoment[", order, "] = ", moments[order]);
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
          agora::pedantic(log_prefix, "Cumulant[1] = ", c1);
          agora::pedantic(log_prefix, "Cumulant[2] = ", c2);
          agora::pedantic(log_prefix, "Cumulant[3] = ", c3);
          agora::pedantic(log_prefix, "Cumulant[4] = ", c4);
          agora::pedantic(log_prefix, "Cumulant[5] = ", c5);
          agora::pedantic(log_prefix, "Cumulant[6] = ", c6);

          // compute sample variance moments
          float k1 = data_test.training_observations.size() - 1;
          float k2 = powf((data_test.training_observations.size() - 1), 2) * (c4 / (data_test.training_observations.size() * powf(c2, 2)) + 2 / (data_test.training_observations.size() - 1));
          float k3 = powf((data_test.training_observations.size() - 1), 3) * (c6 / powf(data_test.training_observations.size(),
                     2) + (12 * c4 * c2) / (data_test.training_observations.size() * (data_test.training_observations.size() - 1)) + (4 * (data_test.training_observations.size() - 2) * powf(c3,
                         2)) / (data_test.training_observations.size() * powf((data_test.training_observations.size() - 1), 2)) + (8 * powf(c2, 3)) / powf((data_test.training_observations.size() - 1), 2)) / powf(c2, 3);
          agora::pedantic(log_prefix, "VarianceMoment[1] = ", k1);
          agora::pedantic(log_prefix, "VarianceMoment[2] = ", k2);
          agora::pedantic(log_prefix, "VarianceMoment[3] = ", k3);

          // compute h0
          data_test.h0 = 1 - ((k1 * k3) / (3 * powf(k2,  2)));
          agora::pedantic(log_prefix, "h0 = ", data_test.h0);

          // compute V(s)=T(S(s)), the power-law transform gaussianization of the sample variance
          // use directly the sample variance w/o gaussianization
          for (auto i : data_test.training_sample_variance)
          {
            agora::pedantic(log_prefix, "Sample Variance = ", i);
            data_test.training_sample_variance_transformed.emplace_back(pow((i / (Parameters_beholder::window_size - 1)), data_test.h0));
            agora::pedantic(log_prefix, "Sample Variance Transformed = ", pow((i / (Parameters_beholder::window_size - 1)), data_test.h0));
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

          agora::pedantic(log_prefix, "Training phase confidence interval for variance: [", data_test.current_variance_conf_interval_lower, ",", data_test.current_variance_conf_interval_upper, "]");

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
      agora::pedantic(log_prefix, "PRODUCTION PHASE, window number: ", data_test.window_number);

      // save the previous sample-mean mean
      float previous_sample_mean_mean = data_test.current_sample_mean_mean;
      //save the previous sample-mean confidence interval (lower and upper)
      float previous_mean_conf_interval_lower = data_test.current_mean_conf_interval_lower;
      float previous_mean_conf_interval_upper = data_test.current_mean_conf_interval_upper;

      // compute the sample mean M(s):
      float sum = 0;

      for (auto i : window_pair)
      {
        sum += i.first;
      }

      float mean = sum / Parameters_beholder::window_size;
      agora::pedantic(log_prefix, "Window mean M(s=",  data_test.window_number, "): ", mean);

      // sample mean mean:
      data_test.current_sample_mean_mean = ((previous_sample_mean_mean * (data_test.window_number - 1)) + mean) / data_test.window_number;
      agora::pedantic(log_prefix, "Current_sample_mean_mean: ", data_test.current_sample_mean_mean);

      // sample mean variance:
      data_test.current_sample_mean_variance = (data_test.reference_sample_mean_variance / sqrtf(data_test.window_number));
      agora::pedantic(log_prefix, "Current_sample_mean_variance: ", data_test.current_sample_mean_variance);

      // compute the confidence interval for the current sample mean
      data_test.current_mean_conf_interval_lower = data_test.current_sample_mean_mean - (Parameters_beholder::gamma_mean * data_test.current_sample_mean_variance);
      data_test.current_mean_conf_interval_upper = data_test.current_sample_mean_mean + (Parameters_beholder::gamma_mean * data_test.current_sample_mean_variance);
      agora::pedantic(log_prefix, "Current window confidence interval for mean: [", data_test.current_mean_conf_interval_lower, ",", data_test.current_mean_conf_interval_upper, "]");

      //int lower_cdt_window = ((window_number * window_size) - (window_size - 1));
      //int upper_cdt_window = (window_number * window_size);

      // compute the intersection between the current confidence interval and the previous one
      // for the lower bound choose the greater of the two
      if (data_test.current_mean_conf_interval_lower < previous_mean_conf_interval_lower)
      {
        data_test.current_mean_conf_interval_lower = previous_mean_conf_interval_lower;
      }

      // for the upper bound choose the lower of the two
      if (data_test.current_mean_conf_interval_upper > previous_mean_conf_interval_upper)
      {
        data_test.current_mean_conf_interval_upper = previous_mean_conf_interval_upper;
      }

      agora::pedantic(log_prefix, "Current intersection confidence interval for mean: [", data_test.current_mean_conf_interval_lower, ",", data_test.current_mean_conf_interval_upper, "]");

      // check whether the intersection of the confidence interval is valid,
      // i.e. if the lower bound is still the lower one and the upper bound is still the upper one
      if (data_test.current_mean_conf_interval_lower > data_test.current_mean_conf_interval_upper)
      {
        change_detected_mean = true;
        agora::info(log_prefix, "CHANGE DETECTED in MEAN, window number ", data_test.window_number);
        agora::pedantic(log_prefix, "between observation number ", ((data_test.window_number * Parameters_beholder::window_size) - (Parameters_beholder::window_size - 1)), " with value: ",
                        window_pair.front().first);
        agora::pedantic(log_prefix, "and observation number ", (data_test.window_number * Parameters_beholder::window_size), " with value: ", window_pair.back().first);

        // convert and save the change window timestamp (as Cassandra does to make it comparable) of the first and last elements of the window
        compute_timestamps(data_test, window_pair);

        return true;
      }

      // as soon as a change is detected the system returns a true, without even checking whether it is for the mean or for the variance
      // thus here I won't even check the variance if the mean already detected a change.
      if (!Parameters_beholder::variance_off && !change_detected_mean)
      {
        // save the previous sample-variance mean
        float previous_sample_variance_mean = data_test.current_sample_variance_mean;
        //save the previous sample-variance confidence interval (lower and upper)
        float previous_variance_conf_interval_lower = data_test.current_variance_conf_interval_lower;
        float previous_variance_conf_interval_upper = data_test.current_variance_conf_interval_upper;

        // compute the sample variance S(s);
        sum = 0;

        for (auto i : window_pair)
        {
          sum += powf((i.first - mean), 2);
        }

        float sample_variance = sum;

        // compute the V(s)=T(S(s))
        float sample_variance_transformed = powf((sample_variance / (Parameters_beholder::window_size - 1)), data_test.h0);

        // sample variance mean:
        data_test.current_sample_variance_mean = ((previous_sample_variance_mean * (data_test.window_number - 1)) + sample_variance_transformed) / data_test.window_number;

        // sample variance variance:
        data_test.current_sample_variance_variance = (data_test.reference_sample_variance_variance / sqrtf(data_test.window_number));

        // compute the confidence interval for the current sample variance
        data_test.current_variance_conf_interval_lower = data_test.current_sample_variance_mean - (Parameters_beholder::gamma_variance * data_test.current_sample_variance_variance);
        data_test.current_variance_conf_interval_upper = data_test.current_sample_variance_mean + (Parameters_beholder::gamma_variance * data_test.current_sample_variance_variance);
        agora::pedantic(log_prefix, "Current window confidence interval for variance: [", data_test.current_variance_conf_interval_lower, ",", data_test.current_variance_conf_interval_upper, "]");


        // compute the intersection between the current confidence interval and the previous one
        // for the lower bound choose the greater of the two
        if (data_test.current_variance_conf_interval_lower < previous_variance_conf_interval_lower)
        {
          data_test.current_variance_conf_interval_lower = previous_variance_conf_interval_lower;
        }

        // for the upper bound choose the lower of the two
        if (data_test.current_variance_conf_interval_upper > previous_variance_conf_interval_upper)
        {
          data_test.current_variance_conf_interval_upper = previous_variance_conf_interval_upper;
        }

        agora::pedantic(log_prefix, "Current intersection confidence interval for variance: [", data_test.current_variance_conf_interval_lower, ",", data_test.current_variance_conf_interval_upper, "]");

        // check whether the intersection of the confidence interval is valid,
        // i.e. if the lower bound is still the lower one and the upper bound is still the upper one
        if (data_test.current_variance_conf_interval_lower > data_test.current_variance_conf_interval_upper)
        {
          change_detected_variance = true;
          agora::info(log_prefix, "CHANGE DETECTED in VARIANCE, window number ", data_test.window_number);
          agora::pedantic(log_prefix, "between observation number ", ((data_test.window_number * Parameters_beholder::window_size) - (Parameters_beholder::window_size - 1)), " with value: ",
                          window_pair.front().first);
          agora::pedantic(log_prefix, "and observation number ", (data_test.window_number * Parameters_beholder::window_size), " with value: ", window_pair.back().first);

          // convert and save the change window timestamp (as Cassandra does to make it comparable) of the first and last elements of the window
          compute_timestamps(data_test, window_pair);

          return true;
        }
      }
    }

    // return true if a change has been detected either in the mean or in the variance
    return change_detected_mean || change_detected_variance;
  }

  // this function writes in the corresponding fields of the "data_test" struct the timestamps of the
  // first and last element of the window passed as the "window_pair" parameter.
  // The peculiarity of this method is the way is converts and saves the timestamps.
  // Basically it converts them in the Cassandra date and time format.
  // In this way we can effortlessly compare the timestamps with the ones from the observations
  // got from queries to Cassandra's db which went through the same kind of processing.
  void IciCdt::compute_timestamps(Data_ici_test& data_test, const std::vector<std::pair <float, std::string>>& window_pair)
  {
    // first window-element timestamp
    // At first the timestamp is a string (as passed from the asrtm) which contains:
    // "seconds(from epoch),nanoseconds". The two fields are separated by a comma.
    // We need to separate these two fields, so we look for the comma.
    const auto front_pos_first_coma = window_pair.front().second.find_first_of(',', 0);
    time_t front_secs_since_epoch;
    int64_t front_nanosecs_since_secs;
    // substring from the beginning of the string to the comma
    std::istringstream( window_pair.front().second.substr(0, front_pos_first_coma) ) >> front_secs_since_epoch;
    // substring from the comma (comma excluded) to the end of the string
    std::istringstream( window_pair.front().second.substr(front_pos_first_coma + 1, std::string::npos) ) >> front_nanosecs_since_secs;

    // now we have to convert them in the funny cassandra format
    data_test.front_year_month_day = cass_date_from_epoch(front_secs_since_epoch);
    data_test.front_time_of_day = cass_time_from_epoch(front_secs_since_epoch);

    // now we add to the time of a day the missing information
    data_test.front_time_of_day += front_nanosecs_since_secs;

    // last window-element timestamp
    // do the same as above
    const auto back_pos_first_coma = window_pair.back().second.find_first_of(',', 0);
    time_t back_secs_since_epoch;
    int64_t back_nanosecs_since_secs;
    std::istringstream( window_pair.back().second.substr(0, back_pos_first_coma) ) >> back_secs_since_epoch;
    std::istringstream( window_pair.back().second.substr(back_pos_first_coma + 1, std::string::npos) ) >> back_nanosecs_since_secs;

    // now we have to convert them in the funny cassandra format
    data_test.back_year_month_day = cass_date_from_epoch(back_secs_since_epoch);
    data_test.back_time_of_day = cass_time_from_epoch(back_secs_since_epoch);

    // now we add to the time of a day the missing information
    data_test.back_time_of_day += back_nanosecs_since_secs;
  }


}
