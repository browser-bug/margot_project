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

#ifndef LAUNCHER_HPP
#define LAUNCHER_HPP

#include <filesystem>
#include <string>

#include "agora/launcher_configuration.hpp"
#include "agora/plugin_configuration.hpp"
#include "agora/agora_properties.hpp"
#include "agora/logger.hpp"

namespace agora {

/**
 * @brief The implementation of a launcher in charge of starting a generic plugin.
 *
 * @details
 * This class represents the entity that starts the execution of a generic plugin. It provides an API that enables the caller to launch the
 * execution in synchronous or asynchronous mode, with a wait function for the latter. Multiple instances of this class are used by a
 * RemoteApplicationHandler to drive the learning process during execution.
 */
class Launcher {
public:
    /**
     * @brief Get a new instance of a Launcher.
     *
     * @param [in] configuration The LauncherConfiguration to use.
     * @param [in] plugin_name The name of the plugin to start with this launcher (i.e. the directory name containing the plugin
     *  implementation).
     *
     * @returns A pointer to the launcher instantiated.
     */
    static std::unique_ptr<Launcher> get_instance(const LauncherConfiguration &configuration, const std::string &plugin_name) {
        return std::unique_ptr<Launcher>(new Launcher(configuration, plugin_name));
    }
    /**
     * @brief Wait the specified plugin execution.
     *
     * @param [in] plugin_pid The process ID corresponding to the plugin execution.
     */
    static void wait(pid_t plugin_pid);

    /**
     * @brief Create a new workspace directory for the specified application.
     *
     * @param [in] app_id The AID corresponding to the application.
     */
    void initialize_workspace(const application_id &app_id);
    /**
     * @brief Delete workspace directory and all its content.
     */
    void clear_workspace();

    /**
     * @brief Launch the managed plugin using the specified environmental configuration.
     *
     * @param [in] env_configuration A data structure containing the environmental parameters to use.
     *
     * @returns The process ID corresponding to the forked process.
     *
     * @details
     * This method forks the caller process and set the last environmental configuration used with the input configuration provided.
     */
    pid_t launch(const PluginConfiguration &env_configuration);
    /**
     * @brief Launch the managed plugin using the last configuration used.
     *
     * @overload
     */
    pid_t launch();

private:
    /**
     * @brief Constructor which uses the configuration provided to set the plugin path and the workspace path.
     *
     * @param [in] configuration The LauncherConfiguration to use.
     */
    Launcher(const LauncherConfiguration &configuration, const std::string &plugin_name);

    /**
     * @brief Write on disk the environmental configuration file with a compatible format.
     *
     * @param [in] env_configuration A datastructure containing the environmental parameters to use.
     * @param [in] config_path The destination path of the environmental configuration file.
     */
    void set_plugin_configuration(const PluginConfiguration &env_configuration, const std::filesystem::path &config_path);
    /**
     * @brief Fork the caller process and start the execution of the managed plugin.
     *
     * @param [in] exec_script_path The path corresponding to the starting script file.
     * @param [in] config_file_path The path corresponding to the environmental configuration file.
     *
     * @returns The process ID corresponding to the forked process.
     */
    pid_t start_plugin(const std::filesystem::path &exec_script_path, const std::filesystem::path &config_file_path) const;
    /**
     * @brief Copy the specified directory to the destination path.
     *
     * @param [in] from The source filesystem path.
     * @param [in] to The destination filesystem path.
     */
    void copy_plugin_directory(const std::filesystem::path &from, const std::filesystem::path &to);

    /**
     * @brief The directory inside the workspace containing the starting script and the plugin files.
     */
    std::filesystem::path plugin_working_dir;
    /**
     * @brief The directory used for sandboxing.
     */
    std::filesystem::path workspace_path;
    /**
     * @brief The directory containing the implementation of the plugin.
     */
    std::filesystem::path plugin_path;

    /**
     * @brief Get the path to the environmental configuration file.
     *
     * @param [in] config_file_name The environmental configuration file name.
     *
     * @returns The filesystem path to the environmental configuration file.
     */
    inline const std::filesystem::path get_config_path(const std::string &config_file_name) const {
        return plugin_working_dir / config_file_name;
    }

    /**
     * @brief The name of the starting script file.
     */
    const std::string script_file_name;
    /**
     * @brief Get the path to the starting script file.
     *
     * @returns The filesystem path to the starting script file.
     */
    inline const std::filesystem::path get_script_path() const { return plugin_working_dir / script_file_name; }

    /**
     * @brief The last environmental configuration file used.
     */
    PluginConfiguration last_env_configuration;

    /**
     * @brief A pointer to the global Logger.
     */
    std::shared_ptr<Logger> logger;
};

}  // namespace agora

#endif // LAUNCHER_HPP
