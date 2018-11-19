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

#include <sstream>
#include <sys/stat.h> // to create directories, only for linux systems
#include <sys/types.h>
#include <sys/wait.h>
#include <cerrno>
#include <unistd.h>
#include <stdexcept>
#include <vector>
#include <fstream>

#include "agora/launcher.hpp"
#include "agora/logger.hpp"
#include "agora/virtual_io.hpp"


namespace agora
{

  /**
   * @NOTE: ALL THE FUNCTION DECLARED IN THIS NAMESPACE SHOULD BE REVRITTEN ONCE WE HAVE std::filesystem
   */
  namespace sh_util
  {


    inline bool create_folder( const std::string& path )
    {
      int rc = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
      return rc == 0 || errno == EEXIST;
    }


    void copy_folder( const std::string& input_folder, const std::string& output_folder )
    {
      // ---..--- create the destination path

      // declare a stringstream to parse all the subfolders
      std::string temporary_directory = "";
      std::stringstream path_stream(output_folder);
      std::string&& current_path = "";

      // loop over the path segment and attempt to create them
      while (std::getline(path_stream, current_path, '/'))
      {
        temporary_directory.append(current_path + "/");
        const bool is_created = create_folder(temporary_directory);

        if (!is_created)
        {
          warning("Launcher: unable to create the folder \"", temporary_directory, "\" with errno=", errno);
          throw std::runtime_error("Launcher: unable to create the folder \"" + temporary_directory + "\" with errno=" + std::to_string(errno) );
        }
      }


      // ---..--- perform the actual copy
      // now we have to recursively copy the folder from the plugin folder
      // the esiest way is to fork and exec cp

      pid_t cp_pid = fork();

      if (cp_pid == 0)
      {
        // we are the child, we need to copy the stuff
        execlp("cp", "cp", "-r", "-T", "-u", input_folder.c_str(), output_folder.c_str(), (char*)NULL);
        warning("Launcher: unable to copy the folder \"", input_folder, "\" into \"", output_folder, "\", errno=", errno);
        throw std::runtime_error( "Launcher: unable to copy the folder \"" + input_folder + "\" into \"" + output_folder + "\", errno=" + std::to_string(errno) );
      }
      else if (cp_pid < 0)
      {
        warning("Launcher: unable to fork errno=", errno);
        throw std::runtime_error( "Launcher: unable to fork errno=" + std::to_string(errno) );
      }

      // wait until it finishes
      int return_code;
      pid_t rc = waitpid(cp_pid, &return_code, 0);

      if (rc < 0)
      {
        warning("Launcher: the \"cp\" process terminated with errno=", errno, ", return code:", return_code);
        throw std::runtime_error( "Launcher: the \"cp\" process terminated with errno=" + std::to_string(errno) + ", return code:" + std::to_string(return_code) );
      }

    }


    void generate_environmental_file( const application_description_t& application, const std::string& destination_file_path, const std::string& metric_name, const std::string& plugin_root_path,
                                      const int iteration_counter )
    {
      std::ofstream config_file;
      config_file.open(destination_file_path, std::ios::out | std::ios::trunc  );
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
      config_file << "DOE_INFO_CONTAINER_NAME=\"" << io::storage.get_doe_info_name(application.application_name) << "\"" << std::endl;
      config_file << "METRIC_NAME=\"" << metric_name << "\"" << std::endl;
      config_file << "METRIC_ROOT=\"" << plugin_root_path << "\"" << std::endl;
      config_file << "ITERATION_COUNTER=\"" << iteration_counter << "\"" << std::endl;
      config_file << "DOE_NAME=\"" << application.doe_name << "\"" << std::endl;
      config_file << "NUMBER_CONFIGURATIONS_PER_ITERATION=\"" << application.number_configurations_per_iteration << "\"" << std::endl;
      config_file << "NUMBER_OBSERVATIONS_PER_CONFIGURATION=\"" << application.number_observations_per_configuration << "\"" << std::endl;
      config_file << "MAX_NUMBER_ITERATION=\"" << application.max_number_iteration << "\"" << std::endl;
      config_file << "MAX_MAE=\"" << application.max_mae << "\"" << std::endl;
      config_file << "MIN_R2=\"" << application.min_r2 << "\"" << std::endl;
      config_file << "VALIDATION_SPLIT=\"" << application.validation_split << "\"" << std::endl;
      config_file << "K_VALUE=\"" << application.k_value << "\"" << std::endl;
      config_file << "MINIMUM_DISTANCE=\"" << application.minimum_distance << "\"" << std::endl;
      if (!application.doe_limits.empty())
      {
        config_file << "DOE_LIMITS=\"" << application.doe_limits << "\"" << std::endl;
      }
      config_file.close();
    }


    pid_t launch_plugin( const std::string& exec_script_path, const std::string& config_file_path )
    {
      pid_t plugin_pid = fork();

      if (plugin_pid == 0)
      {
        // we are the child, we need to execute the pluging
        execlp(exec_script_path.c_str(), exec_script_path.c_str(), config_file_path.c_str(), (char*)NULL);
        warning("Launcher: unable to exec the script \"", exec_script_path, "\", errno=", errno);
        throw std::runtime_error( "Launcher: unable to exec the script \"" + exec_script_path + "\", errno=" + std::to_string(errno) );
      }
      else if (plugin_pid < 0)
      {
        warning("Launcher: unable to fork for execuing the plugin errno=", errno);
        throw std::runtime_error( "Launcher: unable to fork for execuing the plugin errno=" + std::to_string(errno) );
      }

      // if we reach this point, everything will be fine
      return plugin_pid;
    }


    void wait_plugin( const pid_t plugin_pid )
    {
      int plugin_return_code = 0;
      auto rc = waitpid(plugin_pid, &plugin_return_code, 0);

      if (rc < 0)
      {
        warning("Launcher: unable to wait the child \"", plugin_pid, "\" errno=", errno);
        throw std::runtime_error( "Launcher: unable to wait the child \"" + std::to_string(plugin_pid) + "\" errno=" + std::to_string(errno));
      }

      if (plugin_return_code != 0)
      {
        warning("Launcher: a plugin process terminated with return code:", plugin_return_code);
        throw std::runtime_error( "Launcher: a plugin process terminated with return code:" + std::to_string(plugin_return_code) );
      }
    }


  } // namespace sh_util

  // this function generates a model of a metric
  template<>
  void Launcher< LauncherType::ModelGenerator >::operator()( const application_description_t& application, const uint_fast32_t iteration_counter ) const
  {
    // declare the vector that holds the pid of the process that compute a metric
    std::vector< pid_t > builders;
    builders.reserve(application.metrics.size());

    // launch the plugin that models the metric in parallel
    for ( const auto& metric : application.metrics )
    {
      // compute the path of the plugin that computes the metric
      const std::string plugin_path = plugins_folder + metric.prediction_method;

      // compute the destination path for the plugin
      const std::string metric_root = workspace_root + "/" + application.application_name + "/model_" + metric.name;

      // compute the destination path for the environmental file and sh plugin
      const std::string config_path = metric_root + "/" + config_file_name;
      const std::string script_path = metric_root + "/" + script_file_name;

      // now we launch the plugin for computing the target metric
      sh_util::copy_folder(plugin_path, metric_root);
      sh_util::generate_environmental_file(application, config_path, metric.name, metric_root, iteration_counter);

      if (io::storage.support_concurrency())
      {
        builders.emplace_back(sh_util::launch_plugin(script_path, config_path));
      }
      else
      {
        sh_util::wait_plugin(sh_util::launch_plugin(script_path, config_path));
      }
    }

    // wait until they finish
    for ( const auto pid : builders )
    {
      sh_util::wait_plugin(pid);
    }
  }


  // this function generates the dse
  template<>
  void Launcher< LauncherType::DoeGenerator >::operator()( const application_description_t& application, const uint_fast32_t iteration_counter ) const
  {
    // compute the path of the plugin that computes the metric
    const std::string plugin_path = plugins_folder + "doe";

    // compute the destination path for the plugin
    const std::string metric_root = workspace_root + "doe";

    // compute the destination path for the environmental file and sh plugin
    const std::string config_path = metric_root + "/" + config_file_name;
    const std::string script_path = metric_root + "/" + script_file_name;

    // compute the dse
    sh_util::copy_folder(plugin_path, metric_root);
    sh_util::generate_environmental_file(application, config_path, "NA", metric_root, iteration_counter);
    sh_util::wait_plugin(sh_util::launch_plugin(script_path, config_path));
  }
}
