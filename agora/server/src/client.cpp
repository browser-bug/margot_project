#include <cstdlib>
#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <thread>

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
  agora::LogLevel min_log_level;
  fs::path log_file;
  int number_of_threads;
};

po::options_description get_options(application_options &app_opts)
{
  po::options_description options;

  po::options_description desc_opts("Required");
  desc_opts.add_options()("help", "Prints usage informations.");

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

  options.add(desc_opts).add(communication_opts);

  return options;
}

inline auto resolve_mqtt_implementation = [](std::string mqtt_implementation) -> agora::RemoteType {
  const std::unordered_map<std::string, agora::RemoteType> mqtt_implementations = {
      {"paho", agora::RemoteType::Paho},
  };

  auto itr = mqtt_implementations.find(mqtt_implementation);
  if (itr != mqtt_implementations.end())
    return itr->second;
  throw std::invalid_argument("Invalid MQTT implementation \"" + mqtt_implementation + "\", should be one of [paho]");
};

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
    cout << "Usage:\nclient [options]\n";
    cout << cmdline_options;
    return EXIT_SUCCESS;
  }

  po::notify(vm);

  // converting path strings to fs::path
  app_opts.mqtt_implementation = resolve_mqtt_implementation(vm["mqtt-implementation"].as<string>());

  if (app_opts.mqtt_qos < 0 || app_opts.mqtt_qos > 2)
  {
    cerr << "Error: invalid MQTT quality of service " << app_opts.mqtt_qos << ", should be [0,2]" << endl;
    return EXIT_FAILURE;
  }

  // create the application manager
  agora::ApplicationManager &app_manager = agora::ApplicationManager::get_instance();

  // setup the logger
  agora::LoggerConfiguration log_config(agora::LogLevel::DEBUG);
  app_manager.setup_logger(log_config);

  auto logger = app_manager.get_logger();

  // create a virtual channel to communicate with the applications
  logger->info("Client main: bootstrap step 1: estabilish a connection with broker");

  agora::RemoteConfiguration remote_config(app_opts.mqtt_implementation);
  remote_config.set_paho_handler_properties("app1^1.0^block1", app_opts.broker_url, app_opts.mqtt_qos, app_opts.broker_username,
                                            app_opts.broker_password, app_opts.broker_certificate, app_opts.client_certificate,
                                            app_opts.client_key);
  app_manager.setup_remote_handler(remote_config);

  auto remote = app_manager.get_remote_handler();

  std::ostringstream ss;
  ss << std::this_thread::get_id();
  std::string thread_id = ss.str();

  remote->subscribe(agora::MESSAGE_HEADER + "/" + thread_id + "/test");
  //remote->send_message({agora::MESSAGE_HEADER + "/system/" + thread_id, "test@Hello from a local client thread."});
  remote->send_message({agora::MESSAGE_HEADER + "/system/" + thread_id, "shutdown"});

  std::chrono::seconds sleep_duration(3);
  while(true)
  {
    agora::message_model new_incoming_message;
    if (!remote->recv_message(new_incoming_message))
      break;
    logger->info("Received new message from server: topic [", new_incoming_message.topic, "] payload [", new_incoming_message.payload, "]");
    std::this_thread::sleep_for(sleep_duration);
    remote->send_message({agora::MESSAGE_HEADER + "/system/" + thread_id, "test@Hello from a local client thread."});
  }

  return EXIT_SUCCESS;
}
