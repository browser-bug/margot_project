#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "agora/application_manager.hpp"
#include "agora/launcher.hpp"

namespace agora {

namespace fs = std::filesystem;

Launcher::Launcher(const LauncherConfiguration &configuration, const std::string &plugin_name)
    : script_file_name(configuration.script_file_name)
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

void Launcher::wait(pid_t plugin_pid) const
{
  int plugin_return_code = 0;
  auto rc = waitpid(plugin_pid, &plugin_return_code, 0);

  if (rc < 0)
  {
    logger->warning("Launcher: unable to wait the child \"", plugin_pid, "\", errno=", errno);
    throw std::runtime_error("Launcher: unable to wait the child \"" + std::to_string(plugin_pid) + "\" errno=" + std::to_string(errno));
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

void Launcher::set_plugin_configuration(const PluginConfiguration &env_configuration, const std::filesystem::path &config_path)
{
  if (!fs::exists(plugin_working_dir))
  {
    logger->warning("Launcher: the plugin working directory doesn't exist. Have you initiliazed the plugin launcher first?");
    throw std::runtime_error("Launcher: the plugin working directory doesn't exist. Have you initiliazed the plugin launcher first?");
  }

  // write the configuration file into the plugin workspace directory
  std::ofstream config_file;
  config_file.open(config_path, std::ios::out | std::ios::trunc);
  config_file << env_configuration.print_properties();

  // add the working directory path and the config_file path
  config_file << "WORKING_DIRECTORY=\"" << plugin_working_dir.string() << "\"\n";
  config_file << "CONFIG_FILE_PATH=\"" << config_path.string() << "\"\n";

  config_file.close();
}

pid_t Launcher::launch(const PluginConfiguration &env_configuration)
{
  // get the required paths to launch the plugin
  const auto script_path = get_script_path();
  const auto config_path = get_config_path(env_configuration.name);
  if (!fs::exists(script_path))
  {
    logger->warning("Launcher: the plugin script file cannot be found. Have you initiliazed the plugin launcher?");
    throw std::runtime_error("Launcher: the plugin script file cannot be found. Have you initiliazed the plugin launcher?");
  }

  set_plugin_configuration(env_configuration, config_path);
  last_env_configuration = env_configuration;

  // now we launch the plugin
  return start_plugin(script_path, config_path);
}

pid_t Launcher::launch()
{
  logger->info("Launcher: launching using the last configuration set.");
  const auto script_path = get_script_path();
  const auto config_path = get_config_path(last_env_configuration.name);
  if (!fs::exists(config_path))
  {
    logger->warning("Launcher: the environmental configuration file cannot be found. Is this the first time you launch this plugin?");
    throw std::runtime_error("Launcher: the environmental configuration file cannot be found. Is this the first time you launch this plugin?");
  }

  // now we launch the plugin
  return start_plugin(script_path, config_path);
}

} // namespace agora
