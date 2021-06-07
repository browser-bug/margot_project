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

#ifndef FS_DOE_HPP
#define FS_DOE_HPP

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <heel/model_block.hpp>

#include "agora/fs_configuration.hpp"
#include "agora/model_doe.hpp"

namespace agora {

/**
 * @brief Interface representing the storage handler in charge of the DOE data.
 *
 * @details
 * This interface implements a factory pattern which enables the user to get a new instance of the class deriving from it depending on the
 * provided configuration.
 */
class FsDoe {
public:
    /**
     * @brief Get a new instance of the storage handler.
     *
     * @param [in] configuration The storage handler FsConfiguration to use.
     *
     * @returns A pointer to the storage handler instantiated.
     */
    static std::unique_ptr<FsDoe> get_instance(const FsConfiguration &configuration);

    /**
     * @brief Store the DOE data depending on the storage implementation.
     *
     * @param [in] app_id The AID corresponding to the application.
     * @param [in] description The application description containing software-knobs, input features and EFPs.
     * @param [in] doe The data structure representing the DOE configurations to store.
     */
    virtual void store_doe(const application_id &app_id, const margot::heel::block_model &description, const doe_model &doe) = 0;
    /**
     * @brief Load the DOE data depending on the storage implementation.
     *
     * @param [in] app_id The AID corresponding to the application.
     * @param [in] description The application description containing software-knobs, input features and EFPs.
     *
     * @returns The data structure representing the DOE configurations.
     */
    virtual doe_model load_doe(const application_id &app_id, const margot::heel::block_model &description) = 0;
    /**
     * @brief Remove all the DOE configurations still available.
     *
     * @param [in] app_id The AID corresponding to the application.
     * @param [in] description The application description containing software-knobs, input features and EFPs.
     */
    virtual void empty_doe_entries(const application_id &app_id, const margot::heel::block_model &description) = 0;

    /**
     * @brief Get the location of the DOE data in storage.
     *
     * @param [in] app_id The AID corresponding to the application.
     *
     * @returns A string representing the DOE configurations location.
     */
    virtual std::string get_doe_name(const application_id &app_id) const = 0;
    /**
     * @brief Get the location of the total configurations data in storage.
     *
     * @param [in] app_id The AID corresponding to the application.
     *
     * @returns A string representing the total configurations location.
     *
     * @details
     * This data represents all the possible configurations available for the current application. That means all the possible combinations
     * of software-knobs we can have.
     */
    virtual std::string get_total_configurations_name(const application_id &app_id) const = 0;

    /**
     * @brief Delete the DOE data and the total configurations in storage.
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

    virtual ~FsDoe() {}

protected:
    /**
     * @brief Constructor which set the configuration to use and gets the pointer to the global Logger.
     *
     * @param [in] configuration The storage handler FsConfiguration to use.
     */
    FsDoe(const FsConfiguration &configuration);
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

#endif // FS_DOE_HPP
