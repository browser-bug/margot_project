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

#ifndef LAUNCHER_CONFIGURATION_HPP
#define LAUNCHER_CONFIGURATION_HPP

#include <filesystem>
#include <string>

namespace agora {

struct LauncherConfiguration
{
  LauncherConfiguration(){}; // TODO: add default paths too?
  LauncherConfiguration(std::filesystem::path plugins_path, std::filesystem::path workspace_path,
                        const std::string &config_file_name = "plugin_config.env", const std::string &script_file_name = "plugin_start.sh")
      : plugins_path(plugins_path), workspace_path(workspace_path), config_file_name(config_file_name), script_file_name(script_file_name)
  {
    if (!std::filesystem::exists(workspace_path) || !std::filesystem::exists(plugins_path))
      throw std::invalid_argument("Launcher configuration: workspace_path and/or plugin_path do not exist.");
  }

  std::filesystem::path plugins_path;
  std::filesystem::path workspace_path;
  std::string config_file_name;
  std::string script_file_name;
};

} // namespace agora

#endif // LAUNCHER_CONFIGURATION_HPP
