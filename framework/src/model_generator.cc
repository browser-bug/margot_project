/* agora/model_generator.cc
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


/**
 * @NOTE: THIS CLASS SHOULD BE REVRITTEN ONCE WE HAVE std::filesystem
 */

#include <sstream>
#include <sys/stat.h> // to create directories, only for linux systems
#include <sys/types.h>
#include <sys/wait.h>
#include <cerrno>
#include <unistd.h>
#include <stdexcept>
#include <fstream>

#include "agora/model_generator.hpp"
#include "agora/logger.hpp"
#include "agora/virtual_io.hpp"


using namespace agora;

inline bool create_folder( const std::string& path )
{
  int rc = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
  return rc == 0 || errno == EEXIST;
}



void ModelGenerator::operator()( const application_description_t& application ) const
{
  // make sure that there are no entries in the doe table
  info("Handler ", application.application_name, ": clearing the doe table");
  io::storage.empty_doe_entries(application.application_name);

  // create the workspace root folder
  std::string application_workspace = workspace_root;

  if (workspace_root.back() != '/')
  {
    application_workspace.append("/");
  }

  std::stringstream path_stream(application.application_name);
  std::string&& current_path = "";

  while (std::getline(path_stream, current_path, '/'))
  {
    application_workspace.append(current_path + "/");
    const bool is_created = create_folder(application_workspace);

    if (!is_created)
    {
      warning("Model generator: unable to create the folder \"", application_workspace, "\" with errno=", errno);
      throw std::runtime_error("Model generator: unable to create the folder \"" + application_workspace + "\" with errno=" + std::to_string(errno) );
    }
  }

  // we want to generate a model for each metric in a indipendent way
  int metric_counter = 0;
  std::vector<pid_t> builders;

  // loop over the metric that must be pridicted
  for ( const auto& metric : application.metrics )
  {
    // compute the root path for the given metric, we don't use the metric name for security reason
    const std::string metric_root = application_workspace + "metric_" + std::to_string(metric_counter++);

    // compute the path of the source plugin to use to compute the model
    const std::string plugin_path = plugins_folder + "/" + metric.prediction_method;

    // now we have to recursively copy the folder from the plugin folder
    // the esiest way is to fork and exec cp
    pid_t cp_pid = fork();

    if (cp_pid == 0)
    {
      // we are the child, we need to copy the stuff
      execlp("cp", "cp", "-r", "-T", "-u", plugin_path.c_str(), metric_root.c_str(), (char*)NULL);
      warning("Model generator: unable to copy the folder \"", plugin_path, "\" into \"", metric_root, "\", errno=", errno);
      throw std::runtime_error( "Model generator: unable to copy the folder \"" + plugin_path + "\" into \"" + metric_root + "\", errno=" + std::to_string(errno) );
    }
    else if (cp_pid < 0)
    {
      warning("Model generator: unable to fork to copy the generator errno=", errno);
      throw std::runtime_error( "Model generator: unable to fork to copy the generator errno=" + std::to_string(errno) );
    }

    // wait until it finishes
    int return_code;
    pid_t rc = waitpid(cp_pid, &return_code, 0);

    if (rc < 0)
    {
      warning("Model generator: the cp process terminated with errno=", errno, ", return code:", return_code);
      throw std::runtime_error( "Model generator: the cp process terminated with errno=" + std::to_string(errno) + ", return code:" + std::to_string(return_code) );
    }

    // now we have to write the information required to get the data
    const std::string config_file_path = metric_root + "/agora_config.env";
    std::ofstream config_file;
    config_file.open(config_file_path, std::ios::out | std::ios::trunc  );
    config_file << "STORAGE_TYPE=\"" << io::storage.get_type() << "\"" << std::endl;
    config_file << "STORAGE_ADDRESS=\"" << io::storage.get_address() << "\"" << std::endl;
    config_file << "STORAGE_USERNAME=\"" << io::storage.get_username() << "\"" << std::endl;
    config_file << "STORAGE_PASSWORD=\"" << io::storage.get_password() << "\"" << std::endl;
    config_file << "APPLICATION_NAME=\"" << application.application_name << "\"" << std::endl;
    config_file << "OBSERVATION_CONTAINER_NAME=\"" << io::storage.get_observation_name(application.application_name) << "\"" << std::endl;
    config_file << "MODEL_CONTAINER_NAME=\"" << io::storage.get_model_name(application.application_name) << "\"" << std::endl;
    config_file << "KNOBS_CONTAINER_NAME=\"" << io::storage.get_knobs_name(application.application_name) << "\"" << std::endl;
    config_file << "FEATURES_CONTAINER_NAME=\"" << io::storage.get_features_name(application.application_name) << "\"" << std::endl;
    config_file << "DOE_CONTAINER_NAME=\"" << io::storage.get_doe_name(application.application_name) << "\"" << std::endl;
    config_file << "METRIC_NAME=\"" << metric.name << "\"" << std::endl;
    config_file << "METRIC_ROOT=\"" << metric_root << "\"" << std::endl;
    config_file.close();

    // starts the builder
    const std::string builder_executable_path = metric_root + "/generate_model.sh";
    pid_t builder_pid = fork();

    if (builder_pid == 0)
    {
      // we are the child, we need to copy the stuff
      execlp(builder_executable_path.c_str(), builder_executable_path.c_str(), config_file_path.c_str(), (char*)NULL);
      warning("Model generator: unable to exec the model builder \"", builder_executable_path, "\", errno=", errno);
      throw std::runtime_error( "Model generator: unable to exec the model builder \"" + builder_executable_path + "\", errno=" + std::to_string(errno) );
    }
    else if (builder_pid < 0)
    {
      warning("Model generator: unable to fork to exec the model errno=", errno);
      throw std::runtime_error( "Model generator: unable to fork to copy the generator errno=" + std::to_string(errno) );
    }

    // store the pid of the new process
    builders.emplace_back(builder_pid);
  }

  // wait until it finish to elaborate
  for ( const auto pid : builders)
  {
    int return_code = 0;
    auto rc = waitpid(pid, &return_code, 0);

    if (rc < 0)
    {
      warning("Model generator: unable to wait the child \"", pid, "\" errno=", errno, ", child return code:", return_code);
      throw std::runtime_error( "Model generator: a builder process terminated with errno=" + std::to_string(errno) + ", return code:" + std::to_string(return_code) );
    }

    if (return_code != 0)
    {
      warning("Model generator: a builder process terminated with return code:", return_code);
      throw std::runtime_error( "Model generator: a builder process terminated with return code:" + std::to_string(return_code) );
    }
  }

}
