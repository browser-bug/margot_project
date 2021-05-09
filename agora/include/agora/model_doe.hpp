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

#ifndef MODEL_DOE_HPP
#define MODEL_DOE_HPP

#include <deque>
#include <unordered_map>
#include <string>

#include "agora/logger.hpp"

namespace agora {

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
 * @brief A data structure representing a software-knobs configuration.
 *
 * @details
 * In table format this can be represented as | config_id | number_of_explorations | knob_1 | ... | knob_n |
 */
typedef struct configuration_model {
    configuration_model() {}
    /**
     * @brief Construct a new software-knobs configuration.
     *
     * @param [in] config_id The unique identification number of the configuration.
     * @param [in] config The configuration values (i.e. a series of software-knobs values).
     * @param [in] num_explorations The number of explorations required for the configuration.
     */
    configuration_model(const std::string &config_id, const configuration_t &config, int num_explorations)
            : configuration_id(config_id), configuration(config), number_of_explorations(num_explorations) {}

    /**
     * @brief Check whether the configuration is empty (i.e. invalid).
     *
     * @returns True if the configuration is empty, False otherwise.
     */
    bool empty() { return configuration.empty(); }

    /// The configuration unique identifier.
    std::string configuration_id;
    /// The configuration values.
    configuration_t configuration;
    /// The number of required explorations.
    int number_of_explorations;
} configuration_model;

/**
 * @brief A data structure representing the output of the DOE plugin.
 *
 * @details
 * The output is seen as a list of software-knobs configurations.
 */
struct doe_model {
    doe_model() {}
    doe_model(const doe_model &other_doe) : required_explorations(other_doe.required_explorations) {}

    doe_model &operator=(const doe_model &ldoe) {
        required_explorations = ldoe.required_explorations;
        return *this;
    }

    /**
     * @brief Add a new configuration.
     *
     * @param [in] config_id The unique identification number of the configuration.
     * @param [in] config The configuration values (i.e. a series of software-knobs values).
     * @param [in] required_number_of_observations The number of explorations required for the configuration.
     */
    void add_config(const std::string &config_id, const configuration_t &config, const int required_number_of_explorations) {
        required_explorations.emplace_front(config_id, config, required_number_of_explorations);
    }

    /**
     * @brief Get the next configuration to explore.
     *
     * @param [in, out] ref The next configuration to explore.
     *
     * @returns True if a configuration is available, False otherwise.
     *
     * @details
     * The next configuration is extracted in a round-robin fashion, exploiting a std::deque.
     */
    bool get_next(configuration_model &ref) {
        // check if we have an empty map or if we reached the end of it
        if (!required_explorations.empty()) {
            ref = required_explorations.front();
            required_explorations.pop_front();

            if (--ref.number_of_explorations > 0) {
                // if we didn't exhaust this configuration, put it back at the end
                required_explorations.push_back(ref);
            }

            return true;
        }

        return false;
    }

    /**
     * @brief Remove all the configurations.
     */
    void clear() { required_explorations.clear(); }

    /**
     * @brief A list of software-knobs configurations.
     */
    std::deque<configuration_model> required_explorations;
};

}  // namespace agora

#endif // MODEL_DOE_HPP
