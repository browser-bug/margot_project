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

#ifndef FS_DESCRIPTION_HPP
#define FS_DESCRIPTION_HPP

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <heel/model_block.hpp>

#include "agora/fs_configuration.hpp"
#include "agora/logger.hpp"

namespace agora {

/**
 * @brief Interface representing the storage handler in charge of the application description data.
 *
 * @details
 * This interface implements a factory pattern which enables the user to get a new instance of the class deriving from it depending on the
 * provided configuration.
 */
class FsDescription {
public:
    /**
     * @brief Get a new instance of the storage handler.
     *
     * @param [in] configuration The storage handler FsConfiguration to use.
     *
     * @returns A pointer to the storage handler instantiated.
     */
    static std::unique_ptr<FsDescription> get_instance(const FsConfiguration &configuration);

    /**
     * @brief Store the description data depending on the storage implementation.
     *
     * @param [in] app_id The AID corresponding to the application.
     * @param [in] description The application description containing software-knobs, input features and EFPs to store.
     */
    virtual void store_description(const application_id &app_id, const margot::heel::block_model &description) = 0;
    /**
     * @brief Load the description data depending on the storage implementation.
     *
     * @param [in] app_id The AID corresponding to the application.
     *
     * @returns The data structure representing the application description.
     */
    virtual margot::heel::block_model load_description(const application_id &app_id) = 0;

    /**
     * @brief Get the location of the software-knobs data.
     *
     * @param [in] app_id The AID corresponding to the application.
     *
     * @returns A string representing the software-knobs location.
     */
    virtual std::string get_knobs_name(const application_id &app_id) const = 0;
    /**
     * @brief Get the location of the input features data.
     *
     * @param [in] app_id The AID corresponding to the application.
     *
     * @returns A string representing the input features location.
     */
    virtual std::string get_features_name(const application_id &app_id) const = 0;
    /**
     * @brief Get the location of the EFPs data.
     *
     * @param [in] app_id The AID corresponding to the application.
     *
     * @returns A string representing the EFPs location.
     */
    virtual std::string get_metrics_name(const application_id &app_id) const = 0;
    /**
     * @brief Get the location of the Agora properties data.
     *
     * @param [in] app_id The AID corresponding to the application.
     *
     * @returns A string representing the Agora properties location.
     */
    virtual std::string get_properties_name(const application_id &app_id) const = 0;
    /**
     * @brief Get the location of the parameters used by the DOE plugin.
     *
     * @param [in] app_id The AID corresponding to the application.
     *
     * @returns A string representing the location of the parameters used by the DOE plugin.
     */
    virtual std::string get_doe_parameters_name(const application_id &app_id) const = 0;
    /**
     * @brief Get the location of the parameters used by the Modelling plugin.
     *
     * @param [in] app_id The AID corresponding to the application.
     * @param [in] metric_name The name of the EFP.
     *
     * @returns A string representing the location of the parameters used by the Modelling plugin.
     */
    virtual std::string get_model_parameters_name(const application_id &app_id, const std::string &metric_name) const = 0;
    /**
     * @brief Get the location of the parameters used by the Clustering plugin.
     *
     * @param [in] app_id The AID corresponding to the application.
     *
     * @returns A string representing the location of the parameters used by the Clustering plugin.
     */
    virtual std::string get_clustering_parameters_name(const application_id &app_id) const = 0;

    /**
     * @brief Delete the description data in storage.
     *
     * @param [in] app_id The AID corresponding to the application.
     */
    virtual void erase(const application_id &app_id) = 0;

    /**
     * @brief Get the storage type used by the implementing class.
     *
     * @returns A string representing the storage type.
     */
    virtual std::string get_type() const = 0;

    virtual ~FsDescription() {}

protected:
    /**
     * @brief Constructor which set the configuration to use and gets the pointer to the global Logger.
     *
     * @param [in] configuration The storage handler FsConfiguration to use.
     */
    FsDescription(const FsConfiguration &configuration);
    /**
     * @brief The last configuration used by the factory method.
     */
    FsConfiguration configuration;

    /**
     * @brief A pointer to the global Logger.
     */
    std::shared_ptr<Logger> logger;
};

}  // namespace agora

#endif // FS_DESCRIPTION_HPP
