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

#ifndef MARGOT_AGORA_MODEL_PREDICTION_HPP
#define MARGOT_AGORA_MODEL_PREDICTION_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include <heel/model_block.hpp>
#include <heel/generator_utils.hpp>

#include "agora/logger.hpp"

namespace agora {

struct metric_value_model {
    metric_value_model() {}
    metric_value_model(const std::string avg, const std::string std) : _avg(avg), _std(std) {}

    std::string _avg;
    std::string _std;
};

// configuation
// | KB1 | KB2 | ... | KBN |
//
// features (optional)
// | FEAT1 | FEAT2 | ... | FEATN
//
// result
// | M1 | M2 | ... | MN
//
// TODO: this can become something more detailed inside heel
using configuration_t = std::unordered_map<std::string, std::string>;
using features_model = std::unordered_map<std::string, std::string>;
using result_model = std::unordered_map<std::string, metric_value_model>;

struct prediction_model {
    // this method adds a prediction
    bool add_result(const std::string &pred_id, const configuration_t &config, const features_model &feat, const result_model &result) {
        if (!feat.empty())
            return !configurations.insert_or_assign(pred_id, config).second || !features.insert_or_assign(pred_id, feat).second ||
                   !predicted_results.insert_or_assign(pred_id, result).second;
        else
            return !configurations.insert_or_assign(pred_id, config).second || !predicted_results.insert_or_assign(pred_id, result).second;
    }

    // this method removes a prediction
    void remove_result(const std::string &prediction_id) {
        configurations.erase(prediction_id);
        features.erase(prediction_id);
        predicted_results.erase(prediction_id);
    }

    void clear() {
        configurations.clear();
        features.clear();
        predicted_results.clear();
    }

    std::unordered_map<std::string, configuration_t> configurations;
    std::unordered_map<std::string, features_model> features;
    std::unordered_map<std::string, result_model> predicted_results;
};

}  // namespace agora

#endif  // MODEL_PREDICTION_HPP
