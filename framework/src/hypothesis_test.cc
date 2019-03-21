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
#include <algorithm>
#include <random>
#include <boost/math/distributions/students_t.hpp>
using boost::math::students_t;

#include "beholder/hypothesis_test.hpp"
#include "beholder/parameters_beholder.hpp"
#include "agora/logger.hpp"

namespace beholder
{

  bool HypTest::perform_hypothesis_test(const std::unordered_map<std::string, residuals_from_trace>& client_residuals_map, const std::string& application_name,
                                        const std::string& client_name, const std::string& application_workspace, const int& suffix_plot, const std::unordered_map<std::string, Data_ici_test>& ici_cdt_map, const int num_clients)
  {

    int clt_sampling_variables = 1000; // number of sampled variables to be collected for each distribution

    // I'll have to go through each metric present in the structure, but as soon as i find a metric
    // which confirms the test I'lclient_residuals_mapl return true without analyzing the others


    // cycle over the metrics available
    for (auto& i : client_residuals_map)
    {
      // Prefix to log strings containing the app name, the client name and the metric name
      std::string log_prefix = "HYP_TEST:" + application_name + ":" + client_name + ":" + i.first + "---";

      float ici_training_mean_range;
      float mean_populations_difference;

      float t_statistic;
      float v_degree_freedom;

      // populations size
      int n1 = i.second.before_change.size();
      agora::debug(log_prefix, "First population sample size: ", n1);

      int n2 = i.second.after_change.size();
      agora::debug(log_prefix, "Second population sample size: ", n2);



      int clt_samples_for_before = n1 * 0.3; // number of values to be sampled and later averaged to form a variable
      int clt_samples_for_after = n2 * 0.3; // number of values to be sampled and later averaged to form a variable

      std::mt19937 generator(std::random_device{}());

      std::vector<float> clt_distribution_before;
      std::vector<float> clt_distribution_after;

      //if (Parameters_beholder::use_clt){
        // apply CLT theorem to distribution of the data before the change
        for (int j = 0; j < clt_sampling_variables; j++){
          std::vector<float> single_variable_before;
          std::uniform_int_distribution<> unif_distrib(0, std::distance(i.second.before_change.begin(), i.second.before_change.end()) - 1);
          //std::sample(i.second.before_change.begin(), i.second.before_change.end(), std::back_inserter(single_variable_before), clt_samples_for_before, generator);
          for (int k = 0; k < clt_samples_for_before; k++){
            single_variable_before.emplace_back(i.second.before_change.at(unif_distrib(generator)));
          }
          float mean = accumulate( single_variable_before.begin(), single_variable_before.end(), 0.0)/single_variable_before.size();
          clt_distribution_before.emplace_back(mean);
        }

        // apply CLT theorem to distribution of the data after the change
        for (int j = 0; j < clt_sampling_variables; j++){
          std::vector<float> single_variable_after;
          std::uniform_int_distribution<> unif_distrib(0, std::distance(i.second.after_change.begin(), i.second.after_change.end()) - 1);
          for (int k = 0; k < clt_samples_for_after; k++){
            single_variable_after.emplace_back(i.second.after_change.at(unif_distrib(generator)));
          }
          float mean = accumulate( single_variable_after.begin(), single_variable_after.end(), 0.0)/single_variable_after.size();
          clt_distribution_after.emplace_back(mean);
        }

        // write to files the CLT transformations
        std::string metric_folder_path = application_workspace + i.first + "/";

        // creation of output file folders (the suffix subdirectory)
        metric_folder_path = metric_folder_path + std::to_string(suffix_plot) + "/";

        // copy the training lines in the output files for the next iteration, with naming siffix++
        // prepare the next files:
        std::fstream current_metric_before_file;
        std::fstream current_metric_after_file;
        std::string file_path_before = metric_folder_path + "before_change_residuals_" + client_name + "_clt.txt";
        std::string file_path_after = metric_folder_path + "after_change_residuals_" + client_name + "_clt.txt";
        current_metric_before_file.open(file_path_before, std::fstream::out);
        current_metric_after_file.open(file_path_after, std::fstream::out);

        if (!current_metric_before_file.is_open())
        {
          agora::warning(log_prefix, "Error: the CLT residuals before the change file has not been created!");
          throw std::runtime_error("Error: the CLT residuals before the change file has not been created!");
        }

        if (!current_metric_after_file.is_open())
        {
          agora::warning(log_prefix, "Error: the CLT residuals after the change file has not been created!");
          throw std::runtime_error("Error: the CLT residuals after the change file has not been created!");
        }

        for (auto& k : clt_distribution_before)
        {
          current_metric_before_file << k << std::endl;
        }

        for (auto& k : clt_distribution_after)
        {
          current_metric_after_file << k << std::endl;
        }

        current_metric_before_file.flush();
        current_metric_before_file.close();
        current_metric_after_file.flush();
        current_metric_after_file.close();
      //}



      if (Parameters_beholder::use_clt){
        n1 = clt_distribution_before.size();
        n2 = clt_distribution_after.size();
        agora::debug(log_prefix, "First population sample size with CLT: ", n1);
        agora::debug(log_prefix, "Second population sample size with CLT: ", n2);
      }

      // first population sample mean
      float x1 = 0;
      if (Parameters_beholder::use_clt){
        for (auto& j : clt_distribution_before)
        {
          x1 += j;
        }
      } else {
        for (auto& j : i.second.before_change)
        {
          x1 += j;
        }
      }
      x1 = x1 / n1;
      agora::debug(log_prefix, "First population sample mean: ", x1);

      // second population sample mean
      float x2 = 0;
      if (Parameters_beholder::use_clt){
        for (auto& j : clt_distribution_after)
        {
          x2 += j;
        }
      } else {
        for (auto& j : i.second.after_change)
        {
          x2 += j;
        }
      }
      x2 = x2 / n2;
      agora::debug(log_prefix, "Second population sample mean: ", x2);

      // first population sample variance
      float s1_2 = 0;
      if (Parameters_beholder::use_clt){
        for (auto& j : clt_distribution_before)
        {
          s1_2 += powf(j - x1, 2);
        }
      } else {
        for (auto& j : i.second.before_change)
        {
          s1_2 += powf(j - x1, 2);
        }
      }
      s1_2 = s1_2 / (n1 - 1);
      agora::debug(log_prefix, "First population sample variance: ", s1_2);

      // second population sample variance
      float s2_2 = 0;
      if (Parameters_beholder::use_clt){
        for (auto& j : clt_distribution_after)
        {
          s2_2 += powf(j - x2, 2);
        }
      } else {
        for (auto& j : i.second.after_change)
        {
          s2_2 += powf(j - x2, 2);
        }
      }
      s2_2 = s2_2 / (n2 - 1);
      agora::debug(log_prefix, "Second population sample variance: ", s2_2);

      if (x1 == 0)
      {
        agora::warning(log_prefix, "The first population (before the change) sample mean is 0!");
      }

      if (x2 == 0)
      {
        agora::warning(log_prefix, "The second population (after the change) sample mean is 0!");
      }

      if (s1_2 == 0)
      {
        agora::warning(log_prefix, "The first population (before the change) sample variance is 0!");
      }

      if (s2_2 == 0)
      {
        agora::warning(log_prefix, "The second population (after the change) sample variance is 0!");
      }

      float temp = (s1_2 / n1) + (s2_2 / n2);

      // t statistic computation
      t_statistic = (x1 - x2) / (sqrtf(temp));
      agora::debug(log_prefix, "T statistic: ", t_statistic);

      if (std::isinf(t_statistic))
      {
        agora::warning(log_prefix, "The t_statistic is infinite. We consider the test positive because the change is deterministic!");
        return true;
      }

      if (std::isnan(t_statistic))
      {
        agora::warning(log_prefix, "The t_statistic is NaN. We consider the test positive because the change is deterministic!");
        return true;
      }

      // degree of freedom associated with the variance estimates
      float v1 = n1 - 1;
      float v2 = n2 - 1;

      // v degree of freedom computation with the Welch–Satterthwaite equation
      v_degree_freedom = powf(temp, 2) / ((powf(s1_2, 2) / (powf(n1, 2) * v1)) + (powf(s2_2, 2) / (powf(n2, 2) * v2)));
      agora::debug(log_prefix, "Degree of freedom: ", v_degree_freedom);

      if (std::isinf(v_degree_freedom))
      {
        agora::warning(log_prefix, "The v_degree_freedom is infinite. We consider the test positive because the change is deterministic!");
        return true;
      }

      if (std::isnan(v_degree_freedom))
      {
        agora::warning(log_prefix, "The v_degree_freedom is NaN. We consider the test positive because the change is deterministic!");
        return true;
      }

      agora::debug(log_prefix, "User-selected alpha: ", Parameters_beholder::alpha);

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
      // Here we have used the absolute value of the t-statistic, because we initially want to know
      // simply whether there is a difference or not (a two-sided test).
      // The Null-hypothesis: there is no difference in means. Reject if complement of CDF for |t| < significance level / 2:
      // The Alternative-hypothesis: there is a difference in means. Reject if complement of CDF for |t| > significance level / 2:
      // In our situation the change is confirmed when the null hypothesis (no change) is rejected,
      // and then the alternative hypothesis is not rejected.

      if (!Parameters_beholder::disable_bonferroni_correction){
        Parameters_beholder::alpha = Parameters_beholder::alpha / num_clients;
        agora::pedantic(log_prefix, "Using Bonferroni correction, the significance level is: ", Parameters_beholder::alpha);
      }


      if (q < Parameters_beholder::alpha / 2)
      {
        if (!Parameters_beholder::disable_cohen_d_effect_size_check){
          float cohen_d = HypTest::compute_cohen_d(n1, n2, x1, x2, s1_2, s2_2);
          agora::debug(log_prefix, "Cohen's D: ", cohen_d);
          if (cohen_d > Parameters_beholder::cohen_d_threshold){
            agora::pedantic(log_prefix, "Critical value [", q, "] is lower than alpha/2 [", Parameters_beholder::alpha / 2, "].");
            agora::debug(log_prefix, "Null hypothesis: Sample 1 Mean == Sample 2 Mean REJECTED.\n(Alternative hypothesis: Sample 1 Mean != Sample 2 Mean ACCEPTED.)");
            agora::info(log_prefix, "HYPOTHESIS TEST, change confirmed on metric: ", i.first, "!");
            return true;
          } else {
            agora::pedantic(log_prefix, "Critical value [", q, "] is lower than alpha/2 [", Parameters_beholder::alpha / 2, "].");
            agora::info(log_prefix, "Hypothesis test confirmed the change, but the difference in means of the two distributions is not practically significant according to Cohen's Effect Size test, so the change is overall REJECTED");
            agora::info(log_prefix, "The Cohen's D [", cohen_d, "] is lower than the threshold [", Parameters_beholder::cohen_d_threshold, "].");
          }
        } else if (Parameters_beholder::use_difference_means_threshold){
          bool above_threshold = false;
          // look for the ici_cdt_map corresponding to the current metric
          auto search = ici_cdt_map.find(i.first);

          if (search == ici_cdt_map.end())
          {
            agora::warning(log_prefix, "Error: ICI data structure for the current metric in analysis not found!");
          }
          ici_training_mean_range = (search->second.reference_mean_conf_interval_upper - search->second.reference_mean_conf_interval_lower);
          mean_populations_difference = abs(x1 - x2);

          if (mean_populations_difference > ici_training_mean_range * Parameters_beholder::means_threshold_multiplier){
            //agora::info(log_prefix, "Skipping hypothesis test and REJECTING the change because the mean_populations_difference [", mean_populations_difference, "] is lower than the ici test training range for the mean [", ici_training_mean_range, "].");
            above_threshold = true;
          }
          if (above_threshold){
            agora::pedantic(log_prefix, "Critical value [", q, "] is lower than alpha/2 [", Parameters_beholder::alpha / 2, "].");
            agora::debug(log_prefix, "Null hypothesis: Sample 1 Mean == Sample 2 Mean REJECTED.\n(Alternative hypothesis: Sample 1 Mean != Sample 2 Mean ACCEPTED.)");
            agora::info(log_prefix, "HYPOTHESIS TEST, change confirmed on metric: ", i.first, "!");
            return true;
          } else {
            agora::pedantic(log_prefix, "Critical value [", q, "] is lower than alpha/2 [", Parameters_beholder::alpha / 2, "].");
            agora::info(log_prefix, "Hypothesis test confirmed the change, but the difference in means of the two distributions is lower than the user set threshold, so the change is overall REJECTED");
            agora::info(log_prefix, "The mean_populations_difference [", mean_populations_difference, "] is lower than the threshold [", ici_training_mean_range*Parameters_beholder::means_threshold_multiplier, "].");
          }
        } else {
          agora::pedantic(log_prefix, "Critical value [", q, "] is lower than alpha/2 [", Parameters_beholder::alpha / 2, "].");
          agora::debug(log_prefix, "Null hypothesis: Sample 1 Mean == Sample 2 Mean REJECTED.\n(Alternative hypothesis: Sample 1 Mean != Sample 2 Mean ACCEPTED.)");
          agora::info(log_prefix, "HYPOTHESIS TEST, change confirmed on metric: ", i.first, "!");
          return true;
        }
      }
      else
      {
        agora::pedantic(log_prefix, "Critical value [", q, "] is greater than alpha/2 [", Parameters_beholder::alpha / 2, "].");
        agora::debug(log_prefix, "Null hypothesis: Sample 1 Mean == Sample 2 Mean ACCEPTED.\n(Alternative hypothesis: Sample 1 Mean != Sample 2 Mean REJECTED.)");
        agora::info(log_prefix, "HYPOTHESIS TEST, change rejected on metric: ", i.first, "!");
      }

    }

    return false;
  }

  // returns the absolute value of Cohen's D
  float HypTest::compute_cohen_d(const int n1, const int n2, const float x1, const float x2, const float s1_2, const float s2_2){
    float pooled_stddev = sqrt((((n1 - 1) * s1_2) + ((n2 - 1) * s2_2)) / (n1 + n2 - 2));
    float d = (x1 - x2) / (pooled_stddev);
    return abs(d);
  }
}
