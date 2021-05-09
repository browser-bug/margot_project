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

#ifndef FS_MODEL_HPP
#define FS_MODEL_HPP

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <heel/model_block.hpp>

#include "agora/agora_properties.hpp"
#include "agora/fs_configuration.hpp"
#include "agora/logger.hpp"

namespace agora {

/**
 * @brief Storage handler in charge of the models persistency, created by the Modelling plugin.
 *
 * @details
 * This class implements a factory pattern which enables the user to get a new instance of the class deriving from it depending on the
 * provided configuration. Unlike the other storage handler interfaces, this doesn't need to be specialized since the model persistency
 * always happens on local disk.
 */
class FsModel {
public:
    /**
     * @brief Get a new instance of the storage handler.
     *
     * @param [in] configuration The storage handler FsConfiguration to use.
     *
     * @returns A pointer to the storage handler instantiated.
     */
    static std::unique_ptr<FsModel> get_instance(const FsConfiguration &configuration) {
        return std::unique_ptr<FsModel>(new FsModel(configuration));
    }

    /**
     * @brief Check the validity of a model for the specified EFP.
     *
     * @param [in] app_id The AID corresponding to the application.
     * @param [in] metric_name The name of the EFP.
     *
     * @returns True if the model location is valid (i.e. the file exists), False otherwise.
     */
    bool is_model_valid(const application_id &app_id, const std::string &metric_name) const {
        return std::filesystem::exists(get_model_name(app_id, metric_name));
    }

    /**
     * @brief Get the models filesystem root path in which they are stored.
     *
     * @param [in] app_id The AID corresponding to the application.
     *
     * @returns A string representing the filesystem path.
     */
    std::string get_models_path(const application_id &app_id) const {
        std::filesystem::path p = model_dir / app_id.path();
        return p.string();
    }
    /**
     * @brief Get the model filesystem path for the specified EFP.
     *
     * @param [in] app_id The AID corresponding to the application.
     * @param [in] metric_name The name of the EFP.
     *
     * @returns A string representing the filesystem path.
     */
    std::string get_model_name(const application_id &app_id, const std::string &metric_name) const {
        std::filesystem::path p = model_dir / app_id.path() / std::string(metric_name + "_model.data");
        return p.string();
    }

    /**
     * @brief Delete all the models in storage.
     *
     * @param [in] app_id The AID corresponding to the application.
     */
    void erase(const application_id &app_id);

private:
    /**
     * @brief Constructor which set the configuration to use and gets the pointer to the global Logger.
     *
     * @param [in] configuration The storage handler FsConfiguration to use.
     */
    FsModel(const FsConfiguration &configuration);

    /**
     * @brief The last configuration used by the factory method.
     */
    FsConfiguration configuration;

    /**
     * @brief The filesystem root path of the directory where the models are being stored.
     */
    std::filesystem::path model_dir;

    /**
     * @brief A pointer to the global Logger.
     */
    std::shared_ptr<Logger> logger;
};

}  // namespace agora

#endif // FS_MODEL_HPP
