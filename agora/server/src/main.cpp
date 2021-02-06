#include <cstdlib>
#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <boost/program_options.hpp>

#include "agora/application_manager.hpp"
#include "agora/threadpool.hpp"

namespace po = boost::program_options;
namespace fs = std::filesystem;
using namespace std;
using namespace margot;

struct application_options
{
  agora::StorageType storage_implementation;
  string storage_address;
  string storage_username;
  string storage_password;

  agora::RemoteType mqtt_implementation;
  string broker_url;
  string broker_username;
  string broker_password;
  string broker_certificate;
  string client_certificate;
  string client_key;
  int mqtt_qos;

  fs::path workspace_dir;
  fs::path plugin_dir;
  fs::path models_dir;
  agora::LoggerType log_implementation;
  agora::LogLevel min_log_level;
  fs::path log_file;
  int number_of_threads;
};

po::options_description get_options(application_options &app_opts)
{
  po::options_description options;

  po::options_description desc_opts("Required");
  desc_opts.add_options()("help", "Prints usage informations.")("workspace-directory", po::value<string>()->required(),
                                                                "Where the application stores logs and temporary files.")
    ( "plugin-directory", po::value<string>()->required(),
      "The directory with all the available plugins that computes the application model.")
    ( "models-directory", po::value<string>()->required(),
      "The directory that will store all the fitted models produced during evaluation.");

  po::options_description storage_opts("Storage settings");
  storage_opts.add_options()("storage-implementation", po::value<string>()->default_value("csv"),
                             "The name of the storage used by agora [csv].")(
      "storage-address", po::value<string>(&app_opts.storage_address)->default_value(""),
      "A reference to the storage, depending on its actual implementation.")(
      "storage-username", po::value<string>(&app_opts.storage_username)->default_value(""),
      "The username for authentication purposes, if any.")("storage-password",
                                                           po::value<string>(&app_opts.storage_password)->default_value(""),
                                                           "The password for authentication purposes, if any.");

  po::options_description communication_opts("Communication settings");
  communication_opts.add_options()("mqtt-implementation", po::value<string>()->default_value("paho"),
                                   "The name of the actual MQTT client used by agora [paho].")(
      "broker-url", po::value<string>(&app_opts.broker_url)->default_value("127.0.0.1:1883"),
      "The url of the MQTT broker.")("broker-username", po::value<string>(&app_opts.broker_username)->default_value(""),
                                     "The username for authentication purposes, if any.")(
      "broker-password", po::value<string>(&app_opts.broker_password)->default_value(""),
      "The password for authentication purposes, if any.")("broker-ca", po::value<string>(&app_opts.broker_certificate)->default_value(""),
                                                           "The path to the broker certificate (e.g. broker.crt), if any.")(
      "client-ca", po::value<string>(&app_opts.client_certificate)->default_value(""),
      "The path to the client certificate (e.g. client.crt), if any.")("client-private-key",
                                                                       po::value<string>(&app_opts.client_key)->default_value(""),
                                                                       "The path to the private key (e.g. client.key), if any.")(
      "qos", po::value<int>(&app_opts.mqtt_qos)->default_value(2), "The MQTT quality of service level [0,2].");

  po::options_description internal_opts("Agora internal settings");
  internal_opts.add_options()("min-log-level", po::value<string>()->default_value("info"),
                              "The minimum level of logging\n[disabled, warning, info, pedantic, debug].")(
      "log-to-file", po::bool_switch(), "Enables the logging to be happening on file.")(
      "log-file", po::value<string>()->default_value("./margot_agora.log"), "The log file path.")(
      "num-threads", po::value<int>(&app_opts.number_of_threads)->default_value(3),
      "The number of workers to process messages.\n NOTE: it is recommended to have at least one worker for each managed application.");

  options.add(desc_opts).add(storage_opts).add(communication_opts).add(internal_opts);

  return options;
}

inline auto resolve_log_level = [](std::string log_level) -> agora::LogLevel {
  static const std::unordered_map<std::string, agora::LogLevel> log_implementations = {
      {"disabled", agora::LogLevel::DISABLED}, {"info", agora::LogLevel::INFO},       {"debug", agora::LogLevel::DEBUG},
      {"pedantic", agora::LogLevel::PEDANTIC}, {"warning", agora::LogLevel::WARNING},
  };

  auto itr = log_implementations.find(log_level);
  if (itr != log_implementations.end())
    return itr->second;
  throw std::invalid_argument("Invalid log level \"" + log_level + "\", should be one of [disabled, warning, info, pedantic, debug]");
};

inline auto resolve_storage_implementation = [](std::string storage_implementation) -> agora::StorageType {
  static const std::unordered_map<std::string, agora::StorageType> storage_implementations = {
      {"csv", agora::StorageType::CSV},
  };

  auto itr = storage_implementations.find(storage_implementation);
  if (itr != storage_implementations.end())
    return itr->second;
  throw std::invalid_argument("Invalid storage implementation \"" + storage_implementation + "\", should be one of [csv]");
};

inline auto resolve_mqtt_implementation = [](std::string mqtt_implementation) -> agora::RemoteType {
  static const std::unordered_map<std::string, agora::RemoteType> mqtt_implementations = {
      {"paho", agora::RemoteType::Paho},
  };

  auto itr = mqtt_implementations.find(mqtt_implementation);
  if (itr != mqtt_implementations.end())
    return itr->second;
  throw std::invalid_argument("Invalid MQTT implementation \"" + mqtt_implementation + "\", should be one of [paho]");
};

// TODO: create a function that takes the options_description and populate each agora configuration without using an external
// application_options
// void set_configurations(const po::options_description &options, agora::LoggerConfiguration &log_config,
// agora::RemoteConfiguration &remote_config, agora::FsConfiguration &fs_config,
// agora::LauncherConfiguration &launcher_config)
//{}

int main(int argc, char *argv[])
{
  // variables to control the application behavior
  application_options app_opts;

  // option parsing
  po::options_description cmdline_options = get_options(app_opts);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, cmdline_options), vm);

  // first we check that help is set, in which case we don't need to notify for missing required options
  if (vm.count("help"))
  {
    cout << "Usage:\nagora --workspace-directory <path> --plugin-directory <path> [options]\n";
    cout << cmdline_options;
    return EXIT_SUCCESS;
  }

  po::notify(vm);

  // converting path strings to fs::path
  app_opts.workspace_dir = fs::path(vm["workspace-directory"].as<string>());
  app_opts.plugin_dir = fs::path(vm["plugin-directory"].as<string>());
  app_opts.models_dir = fs::path(vm["models-directory"].as<string>());

  app_opts.log_implementation = agora::LoggerType::Console;
  if (vm["log-to-file"].as<bool>())
  {
    app_opts.log_implementation = agora::LoggerType::File;
    app_opts.log_file = fs::path(vm["log-file"].as<string>());
  }

  app_opts.min_log_level = resolve_log_level(vm["min-log-level"].as<string>());
  app_opts.mqtt_implementation = resolve_mqtt_implementation(vm["mqtt-implementation"].as<string>());
  app_opts.storage_implementation = resolve_storage_implementation(vm["storage-implementation"].as<string>());

  if (app_opts.mqtt_qos < 0 || app_opts.mqtt_qos > 2)
  {
    cerr << "Error: invalid MQTT quality of service " << app_opts.mqtt_qos << ", should be [0,2]" << endl;
    return EXIT_FAILURE;
  }
  if (app_opts.number_of_threads < 0)
  {
    std::cerr << "Error: invalid number of threads " << app_opts.number_of_threads << ", cannot be negative" << std::endl;
    return EXIT_FAILURE;
  }

  // create the application manager
  agora::ApplicationManager &app_manager = agora::ApplicationManager::get_instance();

  // setup the logger
  agora::LoggerConfiguration log_config(app_opts.min_log_level, app_opts.log_implementation);
  if(vm["log-to-file"].as<bool>())
  {
    log_config.set_file_logger_properties(app_opts.log_file);
  }
  app_manager.setup_logger(log_config);

  auto logger = app_manager.get_logger();

  // create a virtual channel to communicate with the applications
  logger->info("Agora main: bootstrap step 1: estabilish a connection with broker");

  agora::RemoteConfiguration remote_config(app_opts.mqtt_implementation);
  remote_config.set_paho_handler_properties("agora", app_opts.broker_url, app_opts.mqtt_qos, app_opts.broker_username,
                                            app_opts.broker_password, app_opts.broker_certificate, app_opts.client_certificate,
                                            app_opts.client_key);
  app_manager.setup_remote_handler(remote_config);

  auto remote = app_manager.get_remote_handler();

  // subscribe to relevant topics
  remote->subscribe(agora::MESSAGE_HEADER + "/+/welcome/#");     // to welcome new applications
  remote->subscribe(agora::MESSAGE_HEADER + "/+/info/#");        // to receive information about the application
  remote->subscribe(agora::MESSAGE_HEADER + "/+/observation/#"); // to receive the observations from the clients
  remote->subscribe(agora::MESSAGE_HEADER + "/+/kia/#");         // to receive kill/bye commands from a client
  remote->subscribe(agora::MESSAGE_HEADER + "/system/#");        // to receive external commands

  // sends a welcome message to clients
  remote->send_message({agora::MESSAGE_HEADER + "/welcome", ""});

  // initialize the virtual fs to store/load the information from hard drive
  logger->info("Agora main: bootstrap step 2: initializing the virtual file system");

  agora::FsConfiguration fs_config;
  fs_config.set_csv_handler_properties(app_opts.storage_address, ',');
  fs_config.set_model_handler_properties(app_opts.models_dir);
  fs_config.cluster_type = app_opts.storage_implementation;
  fs_config.description_type = app_opts.storage_implementation;
  fs_config.prediction_type = app_opts.storage_implementation;
  fs_config.observation_type = app_opts.storage_implementation;
  fs_config.doe_type = app_opts.storage_implementation;

  app_manager.set_filesystem_configuration(fs_config);

  // initialize the virtual fs to store/load the information from hard drive
  logger->info("Agora main: bootstrap step 3: initializing the model builder plugin");
  std::filesystem::create_directories(app_opts.workspace_dir);
  agora::LauncherConfiguration launcher_config(app_opts.plugin_dir, app_opts.workspace_dir);
  app_manager.set_launcher_configuration(launcher_config);

  // start the thread pool of worker that manage the plugin_dir
  logger->info("Agora main: bootstrap step 4: hiring ", app_opts.number_of_threads, " oompa loompas");
  agora::ThreadPool workers(app_opts.number_of_threads);
  workers.start_workers();

  // wain until the workers have done
  logger->info("Agora main: bootstrap complete, waiting for workers to finish");
  workers.wait_workers();

  // ok, the whole server is down, time to go out of business
  logger->info("Agora main: all the workers have joined me, farewell my friend");

  return EXIT_SUCCESS;
}
