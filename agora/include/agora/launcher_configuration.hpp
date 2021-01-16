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
