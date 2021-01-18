#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "agora/application_manager.hpp"
#include "agora/launcher.hpp"

namespace agora {

namespace fs = std::filesystem;

Launcher::Launcher(const LauncherConfiguration &configuration, const std::string &plugin_name)
    : config_file_name(configuration.config_file_name), script_file_name(configuration.script_file_name)
{
  ApplicationManager &am = ApplicationManager::get_instance();
  logger = am.get_logger();

  workspace_path = configuration.workspace_path;
  plugin_path = configuration.plugins_path / plugin_name;
}

pid_t Launcher::start_plugin(const fs::path &exec_script_path, const fs::path &config_file_path) const
{
  pid_t plugin_pid = fork();

  if (plugin_pid == 0)
  {
    // we are the child, we need to execute the plugin
    execlp(exec_script_path.c_str(), exec_script_path.c_str(), config_file_path.c_str(), (char *)NULL);
    logger->warning("Launcher: unable to exec the script \"", exec_script_path.string(), "\", errno=", errno);
    throw std::runtime_error("Launcher: unable to exec the script \"" + exec_script_path.string() + "\", errno=" + std::to_string(errno));
  } else if (plugin_pid < 0)
  {
    logger->warning("Launcher: unable to fork for executing the plugin, errno=", errno);
    throw std::runtime_error("Launcher: unable to fork for executing the plugin, errno=%d" + std::to_string(errno));
  }

  // if we reach this point, everything will be fine
  return plugin_pid;
}

void Launcher::wait() const
{
  int plugin_return_code = 0;
  auto rc = waitpid(current_pid, &plugin_return_code, 0);

  if (rc < 0)
  {
    logger->warning("Launcher: unable to wait the child \"", current_pid, "\", errno=", errno);
    throw std::runtime_error("Launcher: unable to wait the child \"" + std::to_string(current_pid) + "\" errno=" + std::to_string(errno));
  }

  if (plugin_return_code != 0)
  {
    logger->warning("Launcher: a plugin process terminated with return code: ", plugin_return_code);
    throw std::runtime_error("Launcher: a plugin process terminated with return code:" + std::to_string(plugin_return_code));
  }
}

void Launcher::copy_plugin_directory(const fs::path &from, const fs::path &to)
{
  // paths sanity checks first
  if (!fs::is_directory(from))
  {
    logger->warning("Launcher: the input path to copy is not a directory ", from.string());
    throw std::runtime_error("Launcher: the input path to copy is not a directory" + from.string());
  }

  // create the destination directory first
  fs::create_directories(to);

  // now we have to recursively copy the folder from the plugin folder
  std::error_code ec;
  const auto copy_options = fs::copy_options::recursive | fs::copy_options::update_existing;
  fs::copy(from, to, copy_options, ec);
  if (ec.value() != 0)
  {
    logger->warning("Launcher: unable to copy the folder \"", from.string(), "\" into \"", to.string(), "\", err=", ec.message());
    throw std::runtime_error("Launcher: unable to copy the folder \"" + from.string() + "\" into \"" + to.string() +
                             "\", err=" + ec.message());
  }
}

void Launcher::initialize_workspace(const application_id &app_id)
{
  // let's copy the plugin directory for sandboxing
  fs::path plugin_destination_path = workspace_path / app_id.path() / plugin_path.filename();

  if (fs::exists(plugin_destination_path))
    logger->warning("Launcher: the pluging working directory already exists.");

  copy_plugin_directory(plugin_path, plugin_destination_path);

  // if everything wents ok, we can set the newly created working directory for the plugin
  plugin_working_dir = plugin_destination_path;
}

void Launcher::set_plugin_configuration(const PluginConfiguration &env_configuration)
{
  if (!fs::exists(plugin_working_dir))
  {
    logger->warning("Launcher: the plugin working directory doesn't exist. Have you initiliazed the plugin launcher first?");
    throw std::runtime_error("Launcher: the plugin working directory doesn't exist. Have you initiliazed the plugin launcher first?");
  }

  // write the configuration file into the plugin workspace directory
  std::ofstream config_file;
  config_file.open(get_config_path(), std::ios::out | std::ios::trunc);
  config_file << env_configuration.print_properties();

  // add the working directory path and the config_file path
  config_file << "WORKING_DIRECTORY=\"" << plugin_working_dir.string() << "\"\n";
  config_file << "CONFIG_FILE_PATH=\"" << get_config_path().string() << "\"\n";

  config_file.close();
}

void Launcher::launch()
{
  // get the required paths to launch the plugin
  const auto script_path = get_script_path();
  const auto config_path = get_config_path();
  if (!fs::exists(script_path) || !fs::exists(config_path))
  {
    logger->warning("Launcher: the plugin sript or the configuration file cannot be found. Have you initiliazed the plugin launcher and "
                    "set a configuration file first?");
    throw std::runtime_error("Launcher: the plugin sript or the configuration file cannot be found. Have you initiliazed the plugin "
                             "launcher and set a configuration file first?");
  }

  // now we launch the plugin and wait for its completition
  current_pid = start_plugin(script_path, config_path);
}

} // namespace agora
