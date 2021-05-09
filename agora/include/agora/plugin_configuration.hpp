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

#ifndef PLUGIN_CONFIGURATION_HPP
#define PLUGIN_CONFIGURATION_HPP

#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <utility>

#include "agora/agora_properties.hpp"

namespace agora {

/**
 * @brief Available plugin implementations.
 *
 * @details
 * These values represents the type of plugin Agora expects, corresponding to each phase of the learning process.
 */
enum class PluginType { DOE, Model, Cluster, Prediction };

/**
 * @brief A generic configuration for a plugin.
 *
 * @details
 * This data structure contains the specification for a generic plugin. This includes a list of properties/parameters which are parsed on
 * the plugin end to infer informations. A PluginConfiguration is packed by the FsHandler before launching the plugin execution.
 */
struct PluginConfiguration {
    PluginConfiguration() {}
    /**
     * @brief Construct a new PluginConfiguration.
     *
     * @param [in] config_name The environmental configuration file name.
     * @param [in] app_id The AID corresponding to the application.
     */
    PluginConfiguration(const std::string &config_name, const application_id &app_id) : config_name(config_name), app_id(app_id) {}
    /**
     * @brief Construct a new PluginConfiguration.
     *
     * @param [in] config_name The environmental configuration file name.
     * @param [in] app_id The AID corresponding to the application.
     * @param [in] metric_name The name of the EFP to model.
     * @param [in] iteration_number The current iteration number of the learning process.
     *
     * @details
     * This constructor is used only for Modelling plugins, specifying the metric to model and the iteration number which are two
     * informations needed inside this type of plugin.
     */
    PluginConfiguration(const std::string &config_name, const application_id &app_id, const std::string &metric_name,
                        const int &iteration_number)
            : config_name(metric_name + "_" + config_name), app_id(app_id), metric_name(metric_name), iteration_number(iteration_number) {}

    /**
     * @brief Get the properties listed in a environmental file compatible format.
     *
     * @returns A string containing the formatted list of properties.
     */
    std::string print_properties() const {
        std::stringstream content;
        for (const auto &pair : this->properties) content << pair.first << "=\"" << pair.second << "\"\n";
        return content.str();
    }

    //TODO: checks if the config list is valid or not
    // bool validate_properties() const {
    //}

    /**
     * @brief A list of properties.
     *
     * @details
     * Each property is seen as a [key, value] pair element where:
     *  - Key: Property name.
     *  - Value: Property value.
     */
    std::map<std::string, std::string> properties;

    std::string config_name;

    application_id app_id;

    // this gets assigned only if we have a configuration for a Model plugin type
    std::string metric_name;
    int iteration_number;
};

}  // namespace agora

#endif // PLUGIN_CONFIGURATION_HPP
