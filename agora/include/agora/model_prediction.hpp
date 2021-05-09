/* Agora library
 * Copyright (C) 2021 Bernardo Menicagli
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

#ifndef MODEL_PREDICTION_HPP
#define MODEL_PREDICTION_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include <heel/model_block.hpp>
#include <heel/generator_utils.hpp>

#include "agora/logger.hpp"

namespace agora {

/**
 * @brief A data structure representing a EFP by its mean value and its standard deviation.
 */
struct metric_value_model {
    metric_value_model() {}
    /**
     * @brief Construct the EFP by specifying its mean and std deviation.
     *
     * @param [in] avg The mean value of the EFP.
     * @param [in] std The standard deviation of the EFP.
     */
    metric_value_model(const std::string avg, const std::string std) : _avg(avg), _std(std) {}

    std::string _avg;
    std::string _std;
};

/**
 * @brief A list of software-knobs values.
 *
 * @details
 * Each element is seen as a (key, value) pair:
 *  - Key: the software-knob name.
 *  - Value: the software-knob value.
 */
using configuration_t = std::unordered_map<std::string, std::string>;
/**
 * @brief A list of input features values.
 *
 * @details
 * Each element is seen as a (key, value) pair:
 *  - Key: the input feature name.
 *  - Value: the input feature value.
 */
using features_model = std::unordered_map<std::string, std::string>;
/**
 * @brief A list of EFPs values.
 *
 * @details
 * Each element is seen as a (key, value) pair:
 *  - Key: the EFP name.
 *  - Value: the EFP value [mean, std_dev].
 */
using result_model = std::unordered_map<std::string, metric_value_model>;

/**
 * @brief A data structure representing the output of the predicting plugin.
 *
 * @details
 * The output is seen as a list of software-knobs configurations, the corresponding input features centroid (if any) and the predicted
 * results (as a pair [mean, std_dev] for each EFP).
 */
struct prediction_model {
    /**
     * @brief Add a new result to the list of predictions.
     *
     * @param [in] pred_id The unique identification number of the result.
     * @param [in] config The configuration values (i.e. a series of software-knobs values).
     * @param [in] feat The input features centroid (i.e. a series of input features values).
     * @param [in] result The result values (i.e. a series of [mean, std_dev] pairs for each EFP).
     *
     * @returns True if the pred_id was seen for the first time, False if the it was already present and hence substituted with a new
     *  value.
     */
    bool add_result(const std::string &pred_id, const configuration_t &config, const features_model &feat, const result_model &result) {
        if (!feat.empty())
            return !configurations.insert_or_assign(pred_id, config).second || !features.insert_or_assign(pred_id, feat).second ||
                   !predicted_results.insert_or_assign(pred_id, result).second;
        else
            return !configurations.insert_or_assign(pred_id, config).second || !predicted_results.insert_or_assign(pred_id, result).second;
    }

    /**
     * @brief Remove the specified result from the list of predictions.
     *
     * @param [in] pred_id The unique identification number of the result.
     */
    void remove_result(const std::string &pred_id) {
        configurations.erase(pred_id);
        features.erase(pred_id);
        predicted_results.erase(pred_id);
    }

    /**
     * @brief Remove all the results from the list of predictions.
     */
    void clear() {
        configurations.clear();
        features.clear();
        predicted_results.clear();
    }

    /**
     * @brief A list of software-knobs configurations.
     *
     * @details
     * Each element is seen as a (key, value) pair:
     *  - Key: the result ID.
     *  - Value: the configuration values.
     */
    std::unordered_map<std::string, configuration_t> configurations;
    /**
     * @brief A list of input features centroids.
     *
     * @details
     * Each element is seen as a (key, value) pair:
     *  - Key: the result ID.
     *  - Value: the centroid values.
     */
    std::unordered_map<std::string, features_model> features;
    /**
     * @brief A list of results.
     *
     * @details
     * Each element is seen as a (key, value) pair:
     *  - Key: the result ID.
     *  - Value: the result values.
     */
    std::unordered_map<std::string, result_model> predicted_results;
};

}  // namespace agora

#endif // MODEL_PREDICTION_HPP
