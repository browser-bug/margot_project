#ifndef MARGOT_AGORA_LAUNCHER_HPP
#define MARGOT_AGORA_LAUNCHER_HPP

#include <filesystem>
#include <string>

#include "agora/launcher_configuration.hpp"
#include "agora/plugin_configuration.hpp"
#include "agora/agora_properties.hpp"
#include "agora/logger.hpp"

namespace agora {

class Launcher {

public:
  static std::unique_ptr<Launcher> get_instance(const LauncherConfiguration &configuration, const std::string& plugin_name)
  {
    return std::unique_ptr<Launcher>(new Launcher(configuration, plugin_name));
  }
  static void wait(pid_t plugin_pid);

  // manage the workspace directory
  void initialize_workspace(const application_id &app_id);
  void clear_workspace();

  // (async) call the plugin launcher with the latest configuration set
  pid_t launch(const PluginConfiguration &env_configuration);
  pid_t launch();

private:
  Launcher(const LauncherConfiguration &configuration, const std::string& plugin_name);

  // sh utility functions
  void set_plugin_configuration(const PluginConfiguration &env_configuration, const std::filesystem::path &config_path);
  pid_t start_plugin(const std::filesystem::path &exec_script_path, const std::filesystem::path &config_file_path) const;
  void copy_plugin_directory(const std::filesystem::path &from, const std::filesystem::path &to);

  // plugin utility paths
  std::filesystem::path plugin_working_dir;
  std::filesystem::path workspace_path;
  std::filesystem::path plugin_path;

  // this is the environmental file that describes the parameters of the agora execution
  inline const std::filesystem::path get_config_path(const std::string &config_file_name) const
  {
    return plugin_working_dir / config_file_name;
  }

  // this is the bash script that launches the actual plugin
  const std::string script_file_name;
  inline const std::filesystem::path get_script_path() const { return plugin_working_dir / script_file_name; }

  PluginConfiguration last_env_configuration;

  std::shared_ptr<Logger> logger;
};

} // namespace agora

#endif // MARGOT_AGORA_LAUNCHER_HPP
