/* beholder/ici_test_data.hpp
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


#ifndef MARGOT_BEHOLDER_ICI_TEST_DATA
#define MARGOT_BEHOLDER_ICI_TEST_DATA

#include "agora/virtual_io.hpp"
#include "beholder/common_objects_beholder.hpp"

namespace beholder
{


  // data structure for the ICI change detection test related to each application-metric pair
  struct Data_ici_test
  {
    // application name
    std::string app_name;

    // metric name
    std::string metric_name;

    // vector to store the observations belonging to the training set
    std::vector<float> training_observations;

    // counter of the observed (complete) windows up to now
    int window_number;

    // bool to check if the variance CI is valid or if it is null (NaN):
    bool valid_variance;

    // training phase data structures:
    std::vector<float> training_sample_mean; // M(s)
    std::vector<float> training_sample_variance; // S(s)
    std::vector<float> training_sample_variance_transformed; // V(s)
    float h0;

    // data for the CDT:
    // in particular the training data is never replaced to allow the reset of the test to
    // its training phase. The training data are all the fields with the "reference" word in them.
    float reference_sample_mean_mean;
    float reference_sample_mean_variance;
    float reference_sample_variance_mean;
    float reference_sample_variance_variance;
    float current_sample_mean_mean;
    float current_sample_mean_variance;
    float current_sample_variance_mean;
    float current_sample_variance_variance;
    float reference_mean_conf_interval_upper;
    float reference_mean_conf_interval_lower;
    float reference_variance_conf_interval_upper;
    float reference_variance_conf_interval_lower;
    float current_mean_conf_interval_upper;
    float current_mean_conf_interval_lower;
    float current_variance_conf_interval_upper;
    float current_variance_conf_interval_lower;
  };

}

#endif // MARGOT_BEHOLDER_ICI_TEST_DATA
