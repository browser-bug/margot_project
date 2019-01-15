/* beholder/main.cc
 * Copyright (C) 2018 Alberto Bendin
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

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <libgen.h>


#include "agora/logger.hpp"
#include "agora/paho_remote_implementation.hpp"
#include "agora/cassandra_fs_implementation.hpp"
#include "agora/virtual_io.hpp"
#include "agora/threadpool.hpp"
#include "beholder/worker_beholder.hpp"
#include "beholder/parameters_beholder.hpp"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

namespace po = boost::program_options;
po::options_description opts_desc("Optional arguments for the Beholder:");
po::variables_map opts_vm;


void ParseCommandLine(int argc, char* argv[])
{
  // Parse command line params
  try
  {
    po::store(po::parse_command_line(argc, argv, opts_desc), opts_vm);
  }
  catch (...)
  {
    std::cout << "Usage: " << argv[0] << " [options]\n";
    std::cout << opts_desc << std::endl;
    ::exit(EXIT_FAILURE);
  }

  po::notify(opts_vm);

  // Check for help request
  if (opts_vm.count("help"))
  {
    std::cout << "Usage: " << argv[0] << " [options]\n";
    std::cout << opts_desc << std::endl;
    ::exit(EXIT_SUCCESS);
  }
}



int main( int argc, char* argv[] )
{
  // variables to control the application behavior
  std::string storage_implementation = "cassandra";
  std::string storage_address = "127.0.0.1";
  std::string storage_username = "";
  std::string storage_password = "";

  std::string mqtt_implementation = "paho";
  std::string broker_url = "127.0.0.1:1883";
  std::string broker_username = "";
  std::string broker_password = "";
  std::string broker_trust_store = "";
  std::string client_certificate = "";
  std::string client_key = "";
  int mqtt_qos = 2;

  std::string workspace_folder;
  std::string plugin_folder;
  std::string min_log_level = "info";
  int number_of_threads = 3;

  char result[ PATH_MAX ];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  const char* path;

  if (count != -1)
  {
    path = dirname(result);
  }

  beholder::Parameters_beholder::workspace_folder = path;

  // Handle the program options
  opts_desc.add_options()
  ("help,h", "print this help message")
  ("storage_implementation", po::value<std::string>(&storage_implementation)->
   default_value(storage_implementation),
   "The name of the actual storage used by beholder (same as agorà). Available alternatives: - \"cassandra\". <str>")
  ("storage_address", po::value<std::string>(&storage_address)->
   default_value(storage_address),
   "A reference to the storage, depending on its actual implementation: for \"cassandra\" the address of a cluster. <str>")
  ("storage_username", po::value<std::string>(&storage_username)->
   default_value(storage_username),
   "The username for authentication purpose, if any. <str>")
  ("storage_password", po::value<std::string>(&storage_password)->
   default_value(storage_password),
   "The password for authentication purpose, if any. <str>")
  ("mqtt_implementation", po::value<std::string>(&mqtt_implementation)->
   default_value(mqtt_implementation),
   "The name of the actual MQTT client used by beholder (same as agorà). Available alternatives: - \"paho\". <str>")
  ("broker_url", po::value<std::string>(&broker_url)->
   default_value(broker_url),
   "The url of the MQTT broker. <str>")
  ("broker_username", po::value<std::string>(&broker_username)->
   default_value(broker_username),
   "The username for authentication purpose, if any. <str>")
  ("broker_password", po::value<std::string>(&broker_password)->
   default_value(broker_password),
   "The password for authentication purpose, if any. <str>")
  ("client_certificate", po::value<std::string>(&client_certificate)->
   default_value(client_certificate),
   "The path to the client certificate (e.g. client.crt), if any. <str>")
  ("client_key", po::value<std::string>(&client_key)->
   default_value(client_key),
   "The path to the private key (e.g. client.key), if any. <str>")
  ("mqtt_qos", po::value<int>(&mqtt_qos)->
   default_value(mqtt_qos),
   "The MQTT quality of service level [0-2]. <int>")
  ("min_log_level", po::value<std::string>(&min_log_level)->
   default_value(min_log_level),
   "The minimum level of logging (stdout). Available alternatives: - \"disabled\" - \"warning\" - \"info\" - \"pedantic\" - \"debug\". <str>")
  ("number_of_threads", po::value<int>(&number_of_threads)->
   default_value(number_of_threads),
   "The number of workers to process messages. NOTE: it is recommended to have at least one worker for each managed application. <int>")
  ("window_size", po::value<int>(&beholder::Parameters_beholder::window_size)->
   default_value(beholder::Parameters_beholder::window_size),
   "The number of observations that fit in a single window of samples. <int>")
  ("training_windows", po::value<int>(&beholder::Parameters_beholder::training_windows)->
   default_value(beholder::Parameters_beholder::training_windows),
   "Number of observation windows to be used as training for the CDT. <int>")
  ("gamma_mean", po::value<float>(&beholder::Parameters_beholder::gamma_mean)->
   default_value(beholder::Parameters_beholder::gamma_mean),
   "Parameter to configure the delay in the detection of the change in the mean. If greater than 1 it delays the change detection reducing the number of false positives. <float>")
  ("gamma_variance", po::value<float>(&beholder::Parameters_beholder::gamma_variance)->
   default_value(beholder::Parameters_beholder::gamma_variance),
   "Parameter to configure the delay in the detection of the change in the variance. If greater than 1 it delays the change detection reducing the number of false positives. <float>")
  ("bad_clients_threshold", po::value<int>(&beholder::Parameters_beholder::bad_clients_threshold)->
   default_value(beholder::Parameters_beholder::bad_clients_threshold),
   "The percentage of clients for every application that is allowed to behave \"badly\" wrt to the model. <int>")
  ("variance_off", po::bool_switch(&beholder::Parameters_beholder::variance_off)->
   default_value(beholder::Parameters_beholder::variance_off),
   "Disables the variance feature from the ICI CDT.")
  ("min_observations", po::value<int>(&beholder::Parameters_beholder::min_observations)->
   default_value(beholder::Parameters_beholder::min_observations),
   "Minimum number of observations (before and after the change window selected in the 1st level of the CDT) to allow the hypothesis test. <int>")
  ("timeout", po::value<int>(&beholder::Parameters_beholder::timeout)->
   default_value(beholder::Parameters_beholder::timeout),
   "Timeout to stop the waiting process during the 2nd level of the CDT for the observations to reach the min_observations number.[Expressed in seconds]. <int>")
  ("frequency_check", po::value<int>(&beholder::Parameters_beholder::frequency_check)->
   default_value(beholder::Parameters_beholder::frequency_check),
   "Frequency of the check for new incoming observations in the trace table. The check will be carried out until either the min_observations number is reached or the wait time runs out according to the timeout.[Expressed in seconds] <int>")
  ("alpha", po::value<float>(&beholder::Parameters_beholder::alpha)->
   default_value(beholder::Parameters_beholder::alpha),
   "Alpha (significance level) used in the hyphotesis test. <float>")
  ("no_trace_drop", po::bool_switch(&beholder::Parameters_beholder::no_trace_drop)->
   default_value(beholder::Parameters_beholder::no_trace_drop),
   "When enabled allows to just delete the trace (after a confirmed change) from the top to the last element of the change window.")
  ("workspace_folder", po::value<std::string>(&beholder::Parameters_beholder::workspace_folder)->
   default_value(beholder::Parameters_beholder::workspace_folder),
   "Absolute path where the application stores temporary files to plot the ICI CDT curves. <path>")
  ("output_files_off", po::bool_switch(&beholder::Parameters_beholder::output_files)->
   default_value(beholder::Parameters_beholder::output_files),
   "Disable the creation of the files needed to plot the ICI curves.")
  ;
  ParseCommandLine(argc, argv);

  // validation of the args
  if (mqtt_qos < 0 || mqtt_qos > 2)
  {
    std::cerr << "Error: invalid MQTT quality of service " << mqtt_qos << ", should be [0,3]" << std::endl;
    return EXIT_FAILURE;
  }

  if (number_of_threads < 0)
  {
    std::cerr << "Error: invalid number of threads " << number_of_threads << ", it cannot be negative" << std::endl;
    return EXIT_FAILURE;
  }

  if (beholder::Parameters_beholder::window_size < 0)
  {
    std::cerr << "Error: invalid observation window size " << beholder::Parameters_beholder::window_size << ", it cannot be negative" << std::endl;
    return EXIT_FAILURE;
  }

  if (beholder::Parameters_beholder::training_windows < 0)
  {
    std::cerr << "Error: invalid training_windows number " << beholder::Parameters_beholder::training_windows << ", it cannot be negative" << std::endl;
    return EXIT_FAILURE;
  }

  if (beholder::Parameters_beholder::gamma_mean < 0)
  {
    std::cerr << "Error: invalid gamma_mean number " << beholder::Parameters_beholder::gamma_mean << ", it cannot be negative" << std::endl;
    return EXIT_FAILURE;
  }

  if (beholder::Parameters_beholder::gamma_variance < 0)
  {
    std::cerr << "Error: invalid gamma_variance number " << beholder::Parameters_beholder::gamma_variance << ", it cannot be negative" << std::endl;
    return EXIT_FAILURE;
  }

  if (beholder::Parameters_beholder::bad_clients_threshold < 0)
  {
    std::cerr << "Error: invalid percentage threshold for bad clients behavior " << beholder::Parameters_beholder::bad_clients_threshold << ", it cannot be negative" << std::endl;
    return EXIT_FAILURE;
  }

  if (beholder::Parameters_beholder::min_observations < 0)
  {
    std::cerr << "Error: invalid min_observations number " << beholder::Parameters_beholder::min_observations << ", it cannot be negative" << std::endl;
    return EXIT_FAILURE;
  }

  if (beholder::Parameters_beholder::timeout < 0)
  {
    std::cerr << "Error: invalid timeout " << beholder::Parameters_beholder::timeout << ", it cannot be negative" << std::endl;
    return EXIT_FAILURE;
  }

  if (beholder::Parameters_beholder::frequency_check < 0)
  {
    std::cerr << "Error: invalid frequency_check " << beholder::Parameters_beholder::frequency_check << ", it cannot be negative" << std::endl;
    return EXIT_FAILURE;
  }

  if (beholder::Parameters_beholder::alpha < 0)
  {
    std::cerr << "Error: invalid alpha " << beholder::Parameters_beholder::alpha << ", it cannot be negative" << std::endl;
    return EXIT_FAILURE;
  }

  if (beholder::Parameters_beholder::workspace_folder != "./workspace/" && beholder::Parameters_beholder::workspace_folder.front() != '/')
  {
    std::cerr << "Error: please use absolute path for the workspace folder" << std::endl;
    return EXIT_FAILURE;
  }

  // set the level of logging
  if (min_log_level.compare("disabled") == 0)
  {
    agora::my_agora_logger.set_filter_at(agora::LogLevel::DISABLED);
  }
  else if (min_log_level.compare("warning") == 0)
  {
    agora::my_agora_logger.set_filter_at(agora::LogLevel::WARNING);
  }
  else if (min_log_level.compare("info") == 0)
  {
    agora::my_agora_logger.set_filter_at(agora::LogLevel::INFO);
  }
  else if (min_log_level.compare("pedantic") == 0)
  {
    agora::my_agora_logger.set_filter_at(agora::LogLevel::PEDANTIC);
  }
  else if (min_log_level.compare("debug") == 0)
  {
    agora::my_agora_logger.set_filter_at(agora::LogLevel::DEBUG);
  }
  else
  {
    std::cerr << "Error: invalid log level \"" << min_log_level << "\", should be on of [disabled, warning, info, pedantic, debug]" << std::endl;
    return EXIT_FAILURE;
  }

  agora::info("Beholder Launched!");

  // create a virtual channel to communicate with the applications and agora
  agora::info("Beholder main: bootstrap step 1: estabilish a connection with broker");

  if ( mqtt_implementation.compare("paho") == 0 )
  {
    agora::io::remote.create<agora::PahoClient>("beholder", broker_url, mqtt_qos, broker_username, broker_password, broker_trust_store, client_certificate, client_key);
  }
  else
  {
    std::cerr << "Error: invalid implementation of MQTT \"" << storage_implementation << "\", available implementations [paho]" << std::endl;
    return EXIT_FAILURE;
  }

  // subscribe to relevant topics
  agora::io::remote.subscribe("margot/+/+/+/model");     // to receive the models from agorà
  agora::io::remote.subscribe("beholder/+/+/+/model");     // to receive the models from agorà (esplicitely addressed to the beholder in reply to the status message for the beholder-agorà sync)
  agora::io::remote.subscribe("margot/+/+/+/kia"); // we are now subscribed to margot/server/kia to receive specific clients kia
  agora::io::remote.subscribe("beholder/+/+/+/observation"); // to receive the observations from the clients
  agora::io::remote.subscribe("margot/system");            // to receive external commands, like "shutdown"
  agora::io::remote.subscribe("margot/agora/kia"); // to handle the absence of agora
  agora::io::remote.subscribe("margot/agora/welcome"); // to handle the presence of agora
  agora::io::remote.subscribe("margot/agora/beholder/welcome"); // to handle the presence of agora (specifically addressed for beholder)

  // initialize the virtual fs to store/load the information from hard drive
  agora::info("Beholder main: bootstrap step 2: initializing the virtual file system");

  if (storage_implementation.compare("cassandra") == 0)
  {
    agora::io::storage.create<agora::CassandraClient>(storage_address, storage_username, storage_password);
  }
  else
  {
    std::cerr << "Error: invalid implementation of the storage \"" << storage_implementation << "\", available implementations [cassandra]" << std::endl;
    return EXIT_FAILURE;
  }

  // start the thread pool of worker that manage the applications
  agora::info("Beholder main: bootstrap step 3: hiring the oompa loompas");
  agora::ThreadPool workers(number_of_threads, beholder::beholder_worker_function);

  // sends the request of summary of current status to agorà
  agora::io::remote.send_message({"beholder/welcome", "Beholder requesting current agora status"});

  // wain until the workers have done
  agora::info("Beholder main: bootstrap complete, waiting for workers to finish");
  workers.wait_workers();

  // ok, the whole server is down, time to go out of business
  agora::info("Beholder main: all the workers have joined me, farewell my friend");

  return EXIT_SUCCESS;
}
