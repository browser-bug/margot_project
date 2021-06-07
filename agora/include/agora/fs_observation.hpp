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

#ifndef FS_OBSERVATION_HPP
#define FS_OBSERVATION_HPP

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <heel/model_block.hpp>

#include "agora/fs_configuration.hpp"
#include "agora/logger.hpp"

namespace agora {

/**
 * @brief Interface representing the storage handler in charge of the observations data.
 *
 * @details
 * This interface implements a factory pattern which enables the user to get a new instance of the class deriving from it depending on the
 * provided configuration.
 */
class FsObservation {
public:
    /**
     * @brief Get a new instance of the storage handler.
     *
     * @param [in] configuration The storage handler FsConfiguration to use.
     *
     * @returns A pointer to the storage handler instantiated.
     */
    static std::unique_ptr<FsObservation> get_instance(const FsConfiguration &configuration);

    /**
     * @brief Create a new container for the observations of the specified application.
     *
     * @param [in] app_id The AID corresponding to the application.
     * @param [in] description The application description containing software-knobs, input features and EFPs to store.
     */
    virtual void create_observation_table(const application_id &app_id, const margot::heel::block_model &description) = 0;
    /**
     * @brief Insert a new observation inside the container.
     *
     * @param [in] app_id The AID corresponding to the application.
     * @param [in] client_id The client identifier.
     * @param [in] duration_sec The observation timestamp in seconds.
     * @param [in] duration_ns The observation timestamp in nanoseconds.
     * @param [in] operating_point The Operating Point representing the observed values and the corresponding configuration.
     */
    virtual void insert_observation_entry(const application_id &app_id, const std::string &client_id, long duration_sec, long duration_ns,
                                          const margot::heel::operating_point_model &operating_point) = 0;

    /**
     * @brief Get the location of the observations data in storage.
     *
     * @param [in] app_id The AID corresponding to the application.
     *
     * @returns A string representing the observations data location.
     */
    virtual std::string get_observation_name(const application_id &app_id) const = 0;

    /**
     * @brief Delete the observations data in storage.
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

    virtual ~FsObservation() {}

protected:
    /**
     * @brief Constructor which set the configuration to use and gets the pointer to the global Logger.
     *
     * @param [in] configuration The storage handler FsConfiguration to use.
     */
    FsObservation(const FsConfiguration &configuration);
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

#endif // FS_OBSERVATION_HPP
