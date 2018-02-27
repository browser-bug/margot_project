/* agora/main.cc
 * Copyright (C) 2018 Davide Gadioli
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
#include <getopt.h>
#include <stdexcept>
#include <sstream>


#include "logger.hpp"
#include "paho_remote_implementation.hpp"
#include "cassandra_fs_implementation.hpp"
#include "virtual_io.hpp"
#include "threadpool.hpp"
#include "worker.hpp"


void print_usage( void )
{
  std::cout << "Usage: agora --workspace_folder <path> --plugin_folder <path> [options]" << std::endl;
  std::cout << "Required arguments:" << std::endl;
  std::cout << " --workspace_folder <path>      Where the application store temporary files to" << std::endl;
  std::cout << "                                build the model (and the logs of the operation)" << std::endl;
  std::cout << " --plugin_folder <path>         The folder with all the available plugins that" << std::endl;
  std::cout << "                                computes the application model" << std::endl << std::endl;
  std::cout << "Optional arguments:" << std::endl;
  std::cout << "--------------------------------------------------------------------------------" << std::endl;
  std::cout << " --storage_implementation <str> The name of the actual storage used by agora" << std::endl;
  std::cout << "                                Available alternatives:" << std::endl;
  std::cout << "                                 - \"cassandra\" [DEFAULT]" << std::endl;
  std::cout << " --storage_address <str>        A reference to the storage, depending on its" << std::endl;
  std::cout << "                                actual implementation:" << std::endl;
  std::cout << "                                 - for \"cassandra\" the address of a cluster" << std::endl;
  std::cout << "                                   DEFAULT = \"127.0.0.1\"" << std::endl;
  std::cout << " --storage_username <str>       The username for authentication purpose, if any" << std::endl;
  std::cout << "                                DEFAULT = \"\"" << std::endl;
  std::cout << " --storage_password <str>       The password for authentication purpose, if any" << std::endl;
  std::cout << "                                DEFAULT = \"\"" << std::endl;
  std::cout << "--------------------------------------------------------------------------------" << std::endl;
  std::cout << " --mqtt_implementation <str>    The name of the actual MQTT client used by agora" << std::endl;
  std::cout << "                                Available alternatives:" << std::endl;
  std::cout << "                                 - \"paho\" [DEFAULT]" << std::endl;
  std::cout << " --broker_url <str>             The url of the MQTT broker" << std::endl;
  std::cout << "                                DEFAULT = \"127.0.0.1:1883\"" << std::endl;
  std::cout << " --broker_username <str>        The username for authentication purpose, if any" << std::endl;
  std::cout << "                                DEFAULT = \"\"" << std::endl;
  std::cout << " --broker_password <str>        The password for authentication purpose, if any" << std::endl;
  std::cout << "                                DEFAULT = \"\"" << std::endl;
  std::cout << " --qos int                      The MQTT quality of service level [0-2]" << std::endl;
  std::cout << "                                DEFAULT = \"2\"" << std::endl;
  std::cout << "--------------------------------------------------------------------------------" << std::endl;
  std::cout << " --min_log_level <str>          The minimum level of logging (stdout)" << std::endl;
  std::cout << "                                Available alternatives:" << std::endl;
  std::cout << "                                 - \"disabled\"" << std::endl;
  std::cout << "                                 - \"warning\"" << std::endl;
  std::cout << "                                 - \"info\"" << std::endl;
  std::cout << "                                 - \"pedantic\"" << std::endl;
  std::cout << "                                 - \"debug\"" << std::endl;
  std::cout << "                                DEFAULT = \"info\"" << std::endl;
  std::cout << " --threads <int>                The number of workers to process messages" << std::endl;
  std::cout << "                                NOTE: it is recommended to have at least one" << std::endl;
  std::cout << "                                worker for each managed application" << std::endl;
  std::cout << "                                DEFAULT = \"3\"" << std::endl;
}


int main( int argc, char* argv[] )
{
  // variables to control the application behavior
  int opt = -1;
  std::string storage_implementation = "cassandra";
  std::string storage_address = "127.0.0.1";
  std::string storage_username = "";
  std::string storage_password = "";

  std::string mqtt_implementation = "paho";
  std::string broker_url = "127.0.0.1:1883";
  std::string broker_username = "";
  std::string broker_password = "";
  int mqtt_qos = 2;

  std::string workspace_folder;
  std::string plugin_folder;
  std::string min_log_level = "info";
  int number_of_threads = 3;

  // parsing the program options
  static struct option long_options[] = {
        {"help",                   no_argument      , 0,  0   },
        {"workspace_folder",       required_argument, 0,  1   },
        {"plugin_folder",          required_argument, 0,  2   },
        {"storage_implementation", required_argument, 0,  3   },
        {"storage_address",        required_argument, 0,  4   },
        {"storage_username",       required_argument, 0,  5   },
        {"storage_password",       required_argument, 0,  6   },
        {"mqtt_implementation",    required_argument, 0,  7   },
        {"broker_url",             required_argument, 0,  8   },
        {"broker_username",        required_argument, 0,  9   },
        {"broker_password",        required_argument, 0,  10   },
        {"qos",                    required_argument, 0,  11   },
        {"min_log_level",          required_argument, 0,  12   },
        {"threads",                required_argument, 0,  13   },
        {0,                        0,                 0,  0   }
  };

  int long_index =0;
  while ((opt = getopt_long(argc, argv,"h:",
                 long_options, &long_index )) != -1)
  {
    switch (opt)
    {
      case 0:
        std::cout << "Agora remote application handler" << std::endl;
        print_usage();
        return EXIT_SUCCESS;
        break;
      case 1:
        if (optarg[0] != '/')
        {
          std::cerr << "Error: please use absolute path for the workspace folder" << std::endl;
          return EXIT_FAILURE;
        }
        workspace_folder = std::string(optarg);
        break;
      case 2:
        if (optarg[0] != '/')
        {
          std::cerr << "Error: please use absolute path for the plugins folder" << std::endl;
          return EXIT_FAILURE;
        }
        plugin_folder = std::string(optarg);
        break;
      case 3:
        storage_implementation = std::string(optarg);
        break;
      case 4:
        storage_address = std::string(optarg);
        break;
      case 5:
        storage_username = std::string(optarg);
        break;
      case 6:
        storage_password = std::string(optarg);
        break;
      case 7:
        mqtt_implementation = std::string(optarg);
        break;
      case 8:
        broker_url = std::string(optarg);
        break;
      case 9:
        broker_username = std::string(optarg);
        break;
      case 10:
        broker_password = std::string(optarg);
        break;
      case 11:
        std::istringstream ( optarg ) >> mqtt_qos;
        if (mqtt_qos < 0 || mqtt_qos > 2)
        {
          std::cerr << "Error: invalid MQTT quality of service " << mqtt_qos << ", should be [0,3]" << std::endl;
          return EXIT_FAILURE;
        }
        break;
      case 12:
        min_log_level = std::string(optarg);
        break;
      case 13:
        std::istringstream ( optarg ) >> number_of_threads;
        if (number_of_threads < 0)
        {
          std::cerr << "Error: invalid number of threads " << number_of_threads << ", it cannot be negative" << std::endl;
          return EXIT_FAILURE;
        }
        break;
      default:
        std::cerr << "Unable to parse the option \"" << optarg << "\"" << std::endl;
        print_usage();
        return EXIT_FAILURE;
    }
  }

  // set the level of logging
  if (min_log_level.compare("disabled") == 0)
  {
    margot::my_agora_logger.set_filter_at(margot::LogLevel::DISABLED);
  }
  else if (min_log_level.compare("warning") == 0)
  {
    margot::my_agora_logger.set_filter_at(margot::LogLevel::WARNING);
  }
  else if (min_log_level.compare("info") == 0)
  {
    margot::my_agora_logger.set_filter_at(margot::LogLevel::INFO);
  }
  else if (min_log_level.compare("pedantic") == 0)
  {
    margot::my_agora_logger.set_filter_at(margot::LogLevel::PEDANTIC);
  }
  else if (min_log_level.compare("debug") == 0)
  {
    margot::my_agora_logger.set_filter_at(margot::LogLevel::DEBUG);
  }
  else
  {
    std::cerr << "Error: invalid log level \"" << min_log_level << "\", should be on of [disabled, warning, info, pedantic, debug]" << std::endl;
    return EXIT_FAILURE;
  }


  // create a virtual channel to communicate with the applications
  margot::info("Agora main: bootstrap step 1: estabilish a connection with broker");
  if ( mqtt_implementation.compare("paho") == 0 )
  {
    margot::io::remote.create<margot::PahoClient>("server", broker_url, mqtt_qos, broker_username, broker_password);
  }
  else
  {
    std::cerr << "Error: invalid implementation of MQTT \"" << storage_implementation << "\", available implementations [paho]" << std::endl;
    return EXIT_FAILURE;
  }

  // subscribe to relevant topics
  margot::io::remote.subscribe("margot/+/+/+/welcome");     // to welcome new applications
  margot::io::remote.subscribe("margot/+/+/+/info");        // to receive information about the application
  margot::io::remote.subscribe("margot/+/+/+/observation"); // to receive the observations from the clients
  margot::io::remote.subscribe("margot/system");            // to receive external commands
  margot::io::remote.subscribe("margot/+/+/+/kia"); // we are not subscribed to margot/server/kia

  // initialize the virtual fs to store/load the information from hard drive
  margot::info("Agora main: bootstrap step 2: initializing the virtual file system");
  if (storage_implementation.compare("cassandra") == 0)
  {
    margot::io::storage.create<margot::CassandraClient>(storage_address, storage_username, storage_password);
  }
  else
  {
    std::cerr << "Error: invalid implementation of the storage \"" << storage_implementation << "\", available implementations [cassandra]" << std::endl;
    return EXIT_FAILURE;
  }


  // initialize the virtual fs to store/load the information from hard drive
  margot::info("Agora main: bootstrap step 3: initializing the model builder plugin");
  margot::io::builder.initialize(workspace_folder, plugin_folder);

  // start the thread pool of worker that manage the applications
  margot::info("Agora main: bootstrap step 4: hiring the oompa loompas");
  margot::ThreadPool workers(number_of_threads, margot::agora_worker_function);


  // wain until the workers have done
  margot::info("Agora main: bootstrap complete, waiting for workers to finish");
  workers.wait_workers();


  // ok, the whole server is down, time to go out of business
  margot::info("Agora main: all the workers have joined me, farewell my friend");

  return EXIT_SUCCESS;
}
