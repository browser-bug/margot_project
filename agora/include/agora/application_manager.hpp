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

#ifndef APPLICATION_MANAGER_HPP
#define APPLICATION_MANAGER_HPP

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "agora/application_handler.hpp"
#include "agora/logger.hpp"
#include "agora/remote_handler.hpp"
#include "agora/fs_handler.hpp"
#include "agora/launcher.hpp"

namespace agora {

/**
 * @brief The implementation of the Application Manager (AM).
 *
 * @details
 * This class is a singleton which is in charge of managing the available resources inside the Agora application. In particular, it stores a
 * list of RemoteApplicationHandler for each application.
 *
 * @note
 * The main methods are mutex protected in order to enforce a consistent internal state.
 */
class ApplicationManager {
public:
    /**
     * @brief Get a singleton instance of this class.
     */
    static ApplicationManager &get_instance() {
        static ApplicationManager am;
        return am;
    }

    /**
     * @brief Create a new Logger instance based on the specified configuration.
     *
     * @param [in] config The Logger configuration to use.
     */
    void setup_logger(const LoggerConfiguration &config) {
        std::lock_guard<std::mutex> lock(global_mutex);
        logger = Logger::get_instance(config);
    }
    /**
     * @brief Create a new RemoteHandler instance based on the specified configuration.
     *
     * @param [in] config The RemoteHandler configuration to use.
     */
    void setup_remote_handler(const RemoteConfiguration &config) {
        std::lock_guard<std::mutex> lock(global_mutex);
        remote = RemoteHandler::get_instance(config);
    }

    /**
     * @brief Set the configuration that the RemoteApplicationHandler will use to create a FsHandler.
     *
     * @param [in] config The FsHandler configuration to use.
     */
    void set_filesystem_configuration(const FsConfiguration &config) { fs_configuration = config; }
    /**
     * @brief Set the configuration that the RemoteApplicationHandler will use to create a plugin Launcher.
     *
     * @param [in] config The plugin Launcher configuration to use.
     */
    void set_launcher_configuration(const LauncherConfiguration &config) { launcher_configuration = config; }

    /**
     * @brief Get the global Logger of Agora.
     *
     * @returns A pointer to the Logger.
     */
    const std::shared_ptr<Logger> get_logger() const { return logger; }

    /**
     * @brief Get the global RemoteHandler of Agora.
     *
     * @returns A pointer to the RemoteHandler.
     */
    const std::shared_ptr<RemoteHandler> get_remote_handler() const { return remote; }

    /**
     * @brief Get the RemoteApplicationHandler for the specified application.
     *
     * @param [in] app_id The unique identifier of the application.
     *
     * @returns A pointer to the RemoteApplicationHandler.
     *
     * @details
     * This method uses lazy instantiation, creating the RemoteApplicationHandler only once if app_id is seen for the first time.
     */
    std::shared_ptr<RemoteApplicationHandler> get_application_handler(const application_id &app_id) {
        std::lock_guard<std::mutex> lock(global_mutex);
        auto iterator = apps.find(app_id.str());

        if (iterator == apps.end()) {
            auto &&application_ptr = std::make_shared<RemoteApplicationHandler>(app_id, fs_configuration, launcher_configuration);
            const auto result_pair = apps.emplace(app_id.str(), application_ptr);

            logger->debug("Creating a new application handler with ID [", app_id.str(), "].");
            return result_pair.first->second;
        }

        return iterator->second;
    }

    /**
     * @brief Remove the RemoteApplicationHandler for the specified application.
     *
     * @param [in] app_id The unique identifier of the application.
     */
    void remove_application_handler(const application_id &app_id) {
        std::lock_guard<std::mutex> lock(global_mutex);
        auto iterator = apps.find(app_id.str());

        if (iterator != apps.end()) {
            apps.erase(iterator);
        } else {
            logger->warning("Couldn't remove application handler: ID not found [", app_id.str(), "].");
        }
    }

private:
    ApplicationManager() {}

    /**
     * @brief The mutex used to enforce a consistent internal state.
     */
    std::mutex global_mutex;

    /**
     * @brief A pointer to the Logger.
     */
    std::shared_ptr<Logger> logger;
    /**
     * @brief A pointer to the RemoteHandler.
     */
    std::shared_ptr<RemoteHandler> remote;
    /**
     * @brief A map of RemoteApplicationHandler pointers.
     *
     * @details
     *  - Key: the application ID.
     *  - Value: a pointer to the corresponding RemoteApplicationHandler.
     */
    std::unordered_map<std::string, std::shared_ptr<RemoteApplicationHandler>> apps;

    /**
     * @brief The FsHandler configuration to use for new instances.
     */
    FsConfiguration fs_configuration;
    /**
     * @brief The plugin Launcher configuration to use for new instances.
     */
    LauncherConfiguration launcher_configuration;
};

}  // namespace agora

#endif // APPLICATION_MANAGER_HPP
