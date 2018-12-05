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
    bool IciCdt::perform_ici_cdt(Data_ici_test& data_test, const std::vector<std::pair <float, std::string>>& window_pair){
        // bool to stop the cycle when a change is detected
        bool change_detected_mean = false;
        bool change_detected_variance = false;

        // bool to know whether or not we are in the training phase
        bool training = true;

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
            for (auto i: window_pair){
                data_test.training_observations.emplace_back(i.first);
            }

            // compute the sample mean and the sample variance for the current window
            // sample mean M(s):
            float sum = 0;
            for (auto i : window_pair){
                sum += i.first;
            }

            float mean = sum / Parameters_beholder::window_size;

            agora::pedantic("Window mean M(s=",  data_test.window_number, "): ", mean);

            // enqueue in the vector the sample mean M(s)
            data_test.training_sample_mean.emplace_back(mean);

            // sample variance S(s):
            if (!Parameters_beholder::variance_off) {
                sum = 0;
                for (auto i : window_pair){
                    sum += powf((i.first - mean), 2);
                }
                data_test.training_sample_variance.emplace_back(sum);
                agora::pedantic("Window variance S(s=",  data_test.window_number, "): ", sum);
            }


            if (data_test.window_number != Parameters_beholder::training_windows){
                // this is not the last training windows
            } else {
                // this is the last training window
            }
        } else {
            // we are in production phase
        }
    }
}
