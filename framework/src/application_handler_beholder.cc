/* beholder/application_handler_beholder.cc
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

#include <ctime>
#include <cassert>
#include <cstdint>
#include <sstream>
#include <algorithm>
#include <limits>
#include <set>
#include <thread>
#include <cmath> // fabs


#include "beholder/application_handler_beholder.hpp"
#include "beholder/observation_data.hpp"
#include "beholder/ici_cdt.hpp"
#include "beholder/hypothesis_test.hpp"
#include "agora/logger.hpp"


using namespace beholder;

RemoteApplicationHandler::RemoteApplicationHandler( const std::string& application_name )
  : status(ApplicationStatus::READY), suffix_plot(1)
{
  // Prefix to log strings containing the application name
  log_prefix = "APP_HANDLER:" + application_name + "---";
  agora::info(log_prefix, "New beholder application handler created for application: ", application_name);

  // load the application description, even though we are just interested in the metrics
  description = agora::io::storage.load_description(application_name);

  // initialize the counters
  current_test_observations_counter = 0;
  observations_counter = 0;
  retraining_counter = 0;
  ici_reset_counter = 0;

  // if the user enabled the exportation of files
  if (Parameters_beholder::output_files)
  {
    // create the workspace root folder
    application_workspace = Parameters_beholder::workspace_folder;

    if (Parameters_beholder::workspace_folder.back() != '/')
    {
      application_workspace.append("/");
    }

    application_workspace.append("workspace_beholder/");
    application_workspace.append(description.application_name);
    application_workspace.append("/");

    std::stringstream path_stream(application_workspace);
    std::string&& current_path = "";

    std::string path_builder;

    while (std::getline(path_stream, current_path, '/'))
    {
      path_builder.append(current_path + "/");
      const bool is_created = create_folder(path_builder);

      if (!is_created)
      {
        agora::warning("Unable to create the folder \"", path_builder, "\" with errno=", errno);
        throw std::runtime_error("Unable to create the folder \"" + path_builder + "\" with errno=" + std::to_string(errno) );
      }
    }

    // write the output file used to plot the training/operational phase windows
    std::ofstream outfileConf (application_workspace + "configTestWindows.txt", std::ofstream::out);

    if (!outfileConf.is_open())
    {
      agora::warning(log_prefix, "Error: the output configuration file has not been created!");
    }

    outfileConf << Parameters_beholder::window_size << " ";
    outfileConf << Parameters_beholder::training_windows << " ";
    outfileConf.close();
  }
}


/**
 * @details
 * This method is the only real "entrance" method of this entire unit.
 * The others are a mere separation of private subfunctions to improve readability all used by this function.
 */
void RemoteApplicationHandler::new_observation( const std::string& values )
{

  // lock the mutex to ensure a consistent global state
  std::unique_lock<std::mutex> guard(mutex);

  // check whether we can analyze the incoming payload or if we need to discard it
  // according to the handler status: ready/computing/disabled/training/retraining
  if ( status != ApplicationStatus::READY )
  {
    if ( status == ApplicationStatus::COMPUTING )
    {
      agora::debug(log_prefix, "Observation DISCARDED since handler is currently performing the second level test (or is however waiting for new observations to come...)!");
    }
    else if ( status == ApplicationStatus::TRAINING)
    {
      agora::debug(log_prefix,
                   "Observation DISCARDED since handler is receiving observations from clients which still have the old invalidated model as knowledge.\nAgorà is working to compute the new model...");
    }
    else if (status == ApplicationStatus::RETRAINING)
    {
      agora::debug(log_prefix,
                   "Observation DISCARDED since handler is receiving observations from clients which still have the old invalidated model as knowledge.\nAgorà is currently down, we cannot do anything about that...");
    }
    else if (status == ApplicationStatus::DISABLED)
    {
      agora::debug(log_prefix, "Observation DISCARDED since Agorà is currently down, we cannot do anything about that...");
    }
    else
    {
      agora::debug(log_prefix, "Observation DISCARDED since handler is not in status READY!");
    }

    return;
  }

  // struct to store the current observation received
  Observation_data observation;

  // parse the received observation
  int parse_outcome = parse_observation(observation, values);

  // if for any reason there is an error in the parsed observation we discard this observation
  if (parse_outcome == 1)
  {
    return;
  }

  // fill in the buffers with the current observations
  int filling_outcome = fill_buffers(observation);

  // if for any reason there is an error in the process of filling the buffers with the residuals
  //from the parsed observation we discard this observation
  if (filling_outcome == 1)
  {
    return;
  }

  agora::pedantic(log_prefix, "Observation successfully parsed and residuals inserted into buffers.");

  // launch the 1st level test.
  // This will only act on the filled-in buffers.
  first_level_test();

  // We only perform the 2nd step of the CDT when the 1st level detects a change.
  // If the change is not detected then the 2nd step is not performed and the method returns
  // and will keep performing the 1st level cdt with the future new observations.

  // STEP 2 of CDT (hypothesis test): analysis with granularity on the single client

  // if need to start the step 2 of CDT:
  if (status == ApplicationStatus::COMPUTING)
  {
    // Local copy of the list of active clients at this moment in time (snapshot of the current situation).
    // So that we have consistency on the clients and their statistics in a situation in which
    // the lock is released, still allowing the real list to be (possibly) updated
    // if some clients send their "kia".
    std::unordered_map<std::string, timestamp_fields> clients_list_snapshot;

    // I am not using the "&" on purpose since I want a copy here!
    for (auto i : clients_list)
    {
      clients_list_snapshot.emplace(i);
    }

    // release the lock to interact with the storage
    guard.unlock();

    // perform the 2nd level test (hypothesis test)
    second_level_test(clients_list_snapshot);
  }

  return;
}


/**
 * @details
 * Method to parse the observation received as a mqtt message payload.
 * Observations coming from blacklisted clients are discarded.
 */
int RemoteApplicationHandler::parse_observation(Observation_data& observation, const std::string& values)
{
  // declare the temporary variables to store the fields of the incoming message
  std::string metrics;
  std::string metric_fields;
  std::string estimates;
  std::string user_enabled_metrics;

  // parse the timestamp and the client_id
  std::stringstream stream(values);
  std::string temp_timestamp;
  stream >> temp_timestamp;
  //agora::debug(log_prefix, "Timestamp: ", temp_timestamp);

  // split the timestamp (comma separated) in seconds and nanoseconds
  auto find_pos_comma = temp_timestamp.find_first_of(",");
  observation.timestamp.seconds = temp_timestamp.substr(0, find_pos_comma);
  observation.timestamp.nanoseconds = temp_timestamp.substr(find_pos_comma + 1, std::string::npos);

  stream >> observation.client_id;
  //agora::debug(log_prefix, "client_id: ", observation.client_id);

  // Let's check whether this is the first observation coming from this client
  auto search_client = clients_list.find(observation.client_id);

  // if this is the first time we encounter this client we need to register it and discard it.
  // This is because we want to avoid the "9999" estimates, that is to say those observations
  // belonging to clients which received the model while a cycle of their computation was
  // already running.
  if (search_client == clients_list.end())
  {
    // emplace in the hashmap the current client with its timestamp struct
    clients_list.emplace(observation.client_id, observation.timestamp);
    agora::info(log_prefix, "Not taking into account this first observation from client ", observation.client_id, " as a security measure for the correctness of the estimates.");
    // return and do not take into account this observation
    return 1;
  }

  // check whether the client which sent the current observation is in the blacklist.
  // if that's the case then discard the observation, otherwise keep going
  auto search = clients_blacklist.find(observation.client_id);

  if (search != clients_blacklist.end())    // if client name found in the blacklist than return
  {
    agora::info(log_prefix, "Observation from client ", observation.client_id, " rejected because blacklisted client");
    return 1;
  }

  // if we arrive here the observation is not blacklisted and is taken into account
  current_test_observations_counter++;
  observations_counter++;

  agora::pedantic(log_prefix, "Current number of observations used in the running ICI test since last reset: ", current_test_observations_counter);
  agora::pedantic(log_prefix, "Observation received from the creation of this handler and used in all the present and past ICI tests: ", observations_counter);
  agora::pedantic(log_prefix, "Total number of times the retraining has been issued by this handler: ", retraining_counter);
  agora::pedantic(log_prefix, "Total number of times the ici test has been reset by this handler: ", ici_reset_counter);

  // get the observed values
  stream >> metrics;
  //agora::debug(log_prefix, "metrics: ", metrics);

  // gets the model values
  stream >> estimates;
  //agora::debug(log_prefix, "estimates: ", estimates );

  // gets the name of the fields of the metric to be filled in.
  // NB: note that the beholder observation message always contains the metric names, also when all the metrics are enabled.
  // This is because in this way we know which metrics must be observed by the beholder
  // according to the user's settings in the XML configuration file.
  // This is needed because there could be metrics which would be available but that should not be monitored
  // by the beholder, again according to the user preferences.
  // The margot heel just sends us the name of the metrics which should be observed.
  // While gathering the metric names from the DB would have meant to lose the information of which metric
  // should be actually monitored out of all those available.
  stream >> metric_fields;
  //agora::debug(log_prefix, "metric_fields: ", metric_fields);

  // if this is the first message then parse also the last part of the message where the heel
  // enqueues the whole list of user-enabled-beholder-metrics.
  // If this list is not present than there is the assumption that all the metrics
  // should be analized, even though the user did not explicitely stated that.
  if (reference_metric_names.size() == 0)
  {
    stream >> user_enabled_metrics;

    // if there is no list than get the reference metrics from the application description,
    // it is assumed that all of them are enabled.
    // This branch should theoretically never be taken because the heel always sends
    // the name of the beholder-enabled-metrics even if they are all enabled (at least as of now).
    if (user_enabled_metrics == "")
    {
      for (auto& i : description.metrics)
      {
        reference_metric_names.emplace(i.name);
      }
    }
    else
    {
      // Separate the parsed information (basically CSV, i.e. strings with comma separated values)
      // and put them into the reference set for the user enabled beholder metrics.
      std::stringstream ssm(user_enabled_metrics);

      while ( ssm.good() )
      {
        std::string substr;
        getline( ssm, substr, ',' );
        reference_metric_names.emplace(substr);
      }
    }
  }

  // Separate the parsed information (basically CSV, i.e. strings with comma separated values)
  // into the respective vectors.
  // Build the vector of metric names provided in the observation.
  std::stringstream ssmf(metric_fields);

  while ( ssmf.good() )
  {
    std::string substr;
    getline( ssmf, substr, ',' );
    observation.metric_fields_vec.push_back( substr );
  }

  // for (auto& i : observation.metric_fields_vec)
  // {
  //   agora::debug(log_prefix, "metric_fields separated: ", i);
  // }

  // build the vector of observed metrics provided in the observation
  std::stringstream ssm(metrics);

  while ( ssm.good() )
  {
    std::string substr;
    getline( ssm, substr, ',' );
    observation.metrics_vec.push_back( std::stof(substr) );
  }

  // for (auto& i : observation.metrics_vec)
  // {
  //   agora::debug(log_prefix, "metrics separated: ", i);
  // }

  // build the vector of observed metrics provided in the observation
  std::stringstream ssme(estimates);

  while ( ssme.good() )
  {
    std::string substr;
    getline( ssme, substr, ',' );
    observation.estimates_vec.push_back( std::stof(substr) );
  }

  // for (auto& i : observation.estimates_vec)
  // {
  //   agora::debug(log_prefix, "estimates separated: ", i);
  // }

  // Check if we have consistency in the quantity of measures received,
  // i.e. if the vector of metric names is as big as the one of the observed metrics and the one of the estimates.
  if ((observation.metric_fields_vec.size() != observation.metrics_vec.size()) || (observation.metrics_vec.size() != observation.estimates_vec.size()))
  {
    agora::warning(log_prefix, "Error in the observation received, mismatch in the number of fields.");
    return 1;
  }

  return 0;
}


/**
 * @details
 * Method which adds the residuals from the current observation to the respective buffers.
 * Every metric has its own buffer.
 * It also writes the residuals to an output file, if the user enabled the file exportation.
 */
int RemoteApplicationHandler::fill_buffers(const Observation_data& observation)
{
  // Insert the residuals in the right buffers according to the metric name
  for (auto index = 0; index < observation.metric_fields_vec.size(); index++)
  {
    agora::pedantic(log_prefix, "Started the process of filling in the buffers with the residuals computed from the parsed observations.");

    // NB: note that the residual is computed with abs()!!
    float current_residual = fabs(observation.estimates_vec[index] - observation.metrics_vec[index]);
    agora::debug(log_prefix, "Current residual for metric ", observation.metric_fields_vec[index], " is: ", current_residual);

    auto search = residuals_map.find(observation.metric_fields_vec[index]);

    if (search != residuals_map.end())
    {
      agora::debug(log_prefix, "Metric ", observation.metric_fields_vec[index], " already present, filling buffer");
      // metric already present, need to add to the buffer the new residual
      residual_struct temp_struct = {current_residual, observation.timestamp};
      search->second.emplace_back(temp_struct);

      // manage the output to file
      if (Parameters_beholder::output_files)
      {
        auto search_file = output_files_map.find(observation.metric_fields_vec[index]);

        // if we arrive here the output file for the current metric should be already available, because it goes 1:1
        // with the respective metric buffer. If that is not the case something is not right...
        if (search_file == output_files_map.end())
        {
          agora::warning(log_prefix, "Error: attempting to write to a file_output_map which does not exist.");
          return 1;
        }

        // if we arrive here (as it is supposed to happen) we need to append the current observation
        // to an already created output-file mapping for the current metric
        search_file->second.observations << current_residual << std::endl;
        search_file->second.observations.flush();
      }
    }
    else
    {
      agora::debug(log_prefix, "Creation of buffer for metric and first insertion: ", observation.metric_fields_vec[index]);
      // need to create the mapping for the current metric. It's the first time you meet this metric
      residual_struct temp_struct = {current_residual, observation.timestamp};
      std::vector<residual_struct> temp_vector;
      temp_vector.emplace_back(temp_struct);
      residuals_map.emplace(observation.metric_fields_vec[index], temp_vector);

      // manage the output to file
      if (Parameters_beholder::output_files)
      {
        auto search_file = output_files_map.find(observation.metric_fields_vec[index]);

        if (search_file != output_files_map.end())
        {
          agora::warning(log_prefix, "Error: attempting the creation of a file_output_map which is already present.");
          return 1;
        }

        // creation of output file folders (the metric subdirectory)
        std::string metric_folder_path = application_workspace + observation.metric_fields_vec[index] + "/";
        bool is_created = create_folder(metric_folder_path);

        if (!is_created)
        {
          agora::warning("Unable to create the folder \"", metric_folder_path, "\" with errno=", errno);
          throw std::runtime_error("Unable to create the folder \"" + metric_folder_path + "\" with errno=" + std::to_string(errno) );
        }

        // creation of output file folders (the suffix subdirectory)
        metric_folder_path = metric_folder_path + std::to_string(suffix_plot) + "/";
        is_created = create_folder(metric_folder_path);

        if (!is_created)
        {
          agora::warning("Unable to create the folder \"", metric_folder_path, "\" with errno=", errno);
          throw std::runtime_error("Unable to create the folder \"" + metric_folder_path + "\" with errno=" + std::to_string(errno) );
        }

        // if we arrive here (as it is supposed to happen) we need to create a new output-file mapping for the current metric
        // current output file
        std::fstream current_metric_observations_file;
        std::fstream current_metric_ici_file;
        std::string file_path_obs = metric_folder_path + "observations_" + observation.metric_fields_vec[index] + "_" + std::to_string(suffix_plot) + ".txt";
        std::string file_path_ici = metric_folder_path + "ici_" + observation.metric_fields_vec[index] + "_" + std::to_string(suffix_plot) + ".txt";
        current_metric_observations_file.open(file_path_obs, std::fstream::in | std::fstream::out | std::fstream::trunc);
        current_metric_ici_file.open(file_path_ici, std::fstream::in | std::fstream::out | std::fstream::trunc);

        if (!current_metric_observations_file.is_open())
        {
          agora::warning(log_prefix, "Error: the output observation file has not been created!");
          throw std::runtime_error("Error: the output observation file cannot be created!");
        }

        if (!current_metric_ici_file.is_open())
        {
          agora::warning(log_prefix, "Error: the output ici file has not been created!");
          throw std::runtime_error("Error: the output ici file cannot be created!");
        }

        current_metric_observations_file << current_residual << std::endl;
        current_metric_observations_file.flush();
        output_files temp_struct_files = {std::move(current_metric_observations_file), std::move(current_metric_ici_file)};
        output_files_map.emplace(observation.metric_fields_vec[index], std::move(temp_struct_files));
      }
    }
  }

  return 0;
}


/**
 * @details
 * Method which carries out the first level ici test.
 * It cycles over all the metrics, and for everyone of these, if the corresponding buffer is
 * filled-in, it performs the test.
 * As soon as the result of the test is positive it breaks out of the cycle and
 * sets the handler status to COMPUTING, thus allowing to go on with the 2nd level test.
 */
void RemoteApplicationHandler::first_level_test( void )
{
  // just universal variables to control the change detection and thus the flow of the CDT itself
  // since we do not care to actually check all the metrics, but it is enough a metric (the first)
  // to trigger the 2nd step of the hierarchiacla CDT.
  bool change_detected = false;

  // Check whether one (or more) buffers is (are) filled in
  // up to the beholder's window_size parameter.
  for (auto& i : residuals_map)
  {
    // if a change has been detected then exit from the for cycle to stop the 1st step of the CDT.
    // Before doing so reset all the buffers for the metrics, whatever their state is.
    if (change_detected)
    {
      agora::debug(log_prefix, "Resetting buffer for metric ", i.first, " after change detected in metric: ", change_metric_name);
      i.second.clear();
      // go to the next metric (next iteration of the for cycle), if available, until the cycle ends
      continue;
    }

    agora::debug(log_prefix, "Current filling level of the buffer for metric ", i.first, ": ", i.second.size(), "/", Parameters_beholder::window_size);

    // if any buffer is filled in then perform the ici cdt on it
    if (i.second.size() == Parameters_beholder::window_size)
    {
      agora::debug(log_prefix, "Buffer for metric ", i.first, " filled in, starting CDT on the current window.");

      // Check if we already have a ici_cdt_map (hashmap data structure) for the current metric, if not create it
      auto search_ici_map = ici_cdt_map.find(i.first);

      // If it finds the ici_cdt_map for the current metric then use it
      if (search_ici_map != ici_cdt_map.end())
      {
        agora::debug(log_prefix, "CDT data for metric ", i.first, " found. Continuing CDT!");
        change_detected = IciCdt::perform_ici_cdt(search_ici_map->second, i.second, output_files_map);
      }
      else
      {
        // It did not find the ici_cdt_map for the current metric then create it.
        // This is basically the first window to be analized for the current metric.
        // Obviously it will be part of the training phase of the ici cdt for that metric.
        Data_ici_test new_ici_struct;
        // save the application name and metric name in the data structure
        // to make those information easily available from the struct once it is passed around (handy in logs)
        new_ici_struct.app_name = description.application_name;
        new_ici_struct.metric_name = i.first;
        // initilize the window number to zero
        new_ici_struct.window_number = 0;
        // initialize the valid_variance to true
        new_ici_struct.valid_variance = true;
        agora::debug(log_prefix, "CDT data for metric ", i.first, " NOT found. Initialized CDT data structure and starting CDT!");
        change_detected = IciCdt::perform_ici_cdt(new_ici_struct, i.second, output_files_map);
        ici_cdt_map.emplace(i.first, new_ici_struct);
        // It cannot detect a change if this is the first full window for that metric, we are in training of cdt
      }

      // if the change has been detected save the name of the (first) metric for which the change has been detected
      // This could be useful later on.
      // Save also the timestamp of the first and last element of the window
      // to be able to pinpoint where the change was detected. This will be needed
      // in the 2nd step of the CDT.
      agora::debug(log_prefix, "Outcome of CDT for metric ", i.first, ": ", change_detected);

      if (change_detected)
      {
        agora::info(log_prefix, "First level of CDT DETECTED A CHANGE in metric: ", i.first, " Resetting metric buffer, clients blacklist and setting handler to status=COMPUTING!");
        change_metric_name = i.first;

        // save the timestamps of the change window to compare them later on with
        // the observations gathered from the trace
        change_window_timestamps.front = i.second.front().residual_timestamp;
        change_window_timestamps.back = i.second.back().residual_timestamp;

        // set the status variable according so that the lock can be released
        status = ApplicationStatus::COMPUTING;
        // every time a new change is detected by the 1st level test we reset the blacklist of clients
        // In this way we check whether they are still behaving badly with the 2nd level test,
        // and we give them the chance to possibly be re-considered as good clients
        // from which to collect observations also for the 1st level test in the future.
        clients_blacklist.clear();
      }

      // Empty the (filled-in) buffer for the current metric.
      i.second.clear();
    }
  }
}


/**
 * @details
 * Performs the 2nd level test, the hypothesis test.
 * First of all it interacts with the storage to gather the information from the trace.
 * It parses the observations from the trace, fills in the buffers and eventually performs the
 * hypothesis test.
 * At the end of the hypothesis test this method determines the fate of the model,
 * meaning that according to the outcome of the test it can invalidate the model and require
 * a retraining or it can reset the ici test (first level test) and keep the current model.
 * It manages the insertion of clients in the blacklist.
 */
void RemoteApplicationHandler::second_level_test( std::unordered_map<std::string, timestamp_fields>& clients_list_snapshot )
{

  agora::info(log_prefix, "Entering 2nd level of CDT!");
  agora::pedantic(log_prefix, "Getting the list of clients running the application...");

  // to store the observations belonging to a pair application-client_name_t
  observations_list_t observations_list;

  // store the list of good-enough clients
  application_list_t good_clients_list;

  // store the list of bad clients for the current observation (not yet blacklisted)
  application_list_t bad_clients_list;

  // clients which did not answer in time because of the timeout
  application_list_t timeout_clients_list;

  // initialize timeout shared (consumed) among all the clients
  int timeout = Parameters_beholder::timeout;

  // cycle over all the clients of the specific application
  for (auto& i : clients_list_snapshot)
  {
    agora::pedantic(log_prefix, "Client: ", i.first, ": Starting the 2nd level of CDT. Setting up the metrics to be analyzed...");

    // initialized as a duplicate of reference_metric_names, that is to say all the metrics enabled for the
    // beholder analysis this list contains the metrics yet to be tested with the hypothesis test, so once a
    // metric has been analyzed it will be removed from this structure
    std::set<std::string> metric_to_be_analyzed;

    for (auto& j : reference_metric_names)
    {
      metric_to_be_analyzed.emplace(j);
      agora::debug(log_prefix, "Client: ", i.first, ": Metric to be analyzed: ", j);
    }

    bool confirmed_change = false;
    bool valid_client = true;

    // String containing the 'select' part of the query to Cassandra
    // to only receive the metrics enabled for the beholder analysis by the user
    std::string query_select;

    // just for log purposes, to keep track of how many times we enter the while cycle
    int while_counter = 1;

    while (metric_to_be_analyzed.size() != 0 && !confirmed_change)
    {
      agora::debug(log_prefix, "Client: ", i.first, ": Entering while cycle for the ", while_counter, " time with still ", metric_to_be_analyzed.size(), " metrics to be analyzed out of a total of ",
                   reference_metric_names.size(), " beholder-enabled metrics.");
      // hashmap to save the residuals for the specific application-client:
      // the key is the name of the metric. The corresponding mapped value is a struct:
      // The first element of the struct is a vector containing the residuals for that metric before the change windows
      // The second element of the struct is a vector containing the residuals for that metric after the change windows
      std::unordered_map<std::string, residuals_from_trace> client_residuals_map;

      // preparing the 'select' query for Cassandra for the 2nd step of cassandra
      // It is done here on the basis of which metrics are left to be analyzed
      query_select = "day, time, client_id, ";

      for (auto& it : description.knobs)
      {
        query_select.append(it.name);
        query_select.append(",");
      }

      for (auto& it : description.features)
      {
        query_select.append(it.name);
        query_select.append(",");
      }

      for (auto& it : metric_to_be_analyzed)
      {
        query_select.append(it);
        query_select.append(",");
      }

      for (auto& it : metric_to_be_analyzed)
      {
        query_select.append("model_");
        query_select.append(it);
        query_select.append(",");
      }

      // to remove the last comma
      query_select.pop_back();

      agora::debug(log_prefix, "Client: ", i.first, ": Querying DB to get its observations from the trace table.");
      //agora::debug(log_prefix, "Query: ", query_select);

      // Gather the rows we are interested in from the trace. We pass the first part of the query (the "select")
      // and a timestamp (in seconds and nanoseconds) from wich to read the trace. We are not interested
      // in the part of the trace with timestamps older than those passed here.
      observations_list = agora::io::storage.load_client_observations(description.application_name, i.first, query_select, i.second.seconds,
                          i.second.nanoseconds);

      agora::debug("\n", log_prefix, "Client: ", i.first, ": Trace query executed successfully, starting the parsing of its observations.");
      agora::debug("\n", log_prefix, "Client: ", i.first, ": Collected ", observations_list.size(), " observations from the trace.");

      // cycle over each row j of the trace for each client i (for the current application)
      // and print a dump of the trace for the current client, if needed
      // for (auto& j : observations_list)
      // {
      //   agora::debug(log_prefix, "Client: ", i.first, ": Observation: ", j);
      // }

      // cycle over each row j of the trace for each client i (for the current application)
      // and insert the observations in the respective buffers
      for (auto& j : observations_list)
      {
        parse_and_insert_observations_for_client_from_trace(client_residuals_map, j, metric_to_be_analyzed);
      }

      agora::debug(log_prefix, "Client: ", i.first, ": Finished parsing and putting in the respective buffers the result of the trace query.");
      agora::debug(log_prefix, "Client: ", i.first, ": Computing which metrics have enough observations to actually perform the hypothesis test.");

      // let's analyze if any of the metrics has enough observations to perform the test and
      // remove the metrics which cannot be analyzed from the client_residuals_map.
      // The user can specify as a beholder cli option which is this minimum number of observations
      // for the two populations (before and after the change) to perform the test.
      for (auto it = client_residuals_map.begin(); it != client_residuals_map.end();)
      {
        if (it->second.before_change.size() < Parameters_beholder::min_observations || it->second.after_change.size() < Parameters_beholder::min_observations)
        {
          agora::debug(log_prefix, "Client: ", i.first, ": Insufficient data [user_requirement: ", Parameters_beholder::min_observations, "] to perform 2nd level hypothesis test on metric ", it->first,
                       ". # observations before the change: ", it->second.before_change.size(), ". # observations after the change: ", it->second.after_change.size());
          it = client_residuals_map.erase(it);
        }
        else
        {
          agora::debug(log_prefix, "Client: ", i.first, ": 2nd level hypothesis test on metric ", it->first,
                       " feasible: # observations before the change: ", it->second.before_change.size(), ". # observations after the change: ", it->second.after_change.size());
          ++it;
        }
      }

      if (client_residuals_map.size() == metric_to_be_analyzed.size())
      {
        agora::debug(log_prefix, "Client: ", i.first, ": All the ", client_residuals_map.size(), " out of ", metric_to_be_analyzed.size(), " metrics to still be analyzed can perform the test.");
      }
      else
      {
        agora::debug(log_prefix, "Client: ", i.first, ": Just ", client_residuals_map.size(), " out of ", metric_to_be_analyzed.size(), " metrics to still be analyzed can perform the test.");
      }

      // if there is at least a metric on which we can perform the hypothesis test
      if (client_residuals_map.size() > 0)
      {
        // export the residuals to file in order to be able to run the hypothesis test manually with the python script provided
        if (Parameters_beholder::output_files)
        {
          for (auto& j : client_residuals_map)
          {
            // the metric subdirectory should have already been created (because we reach this point only if the
            // structure of the metric in analysis has been created), but there should be theoretically
            // no harm in making sure about that.
            std::string metric_folder_path = application_workspace + j.first + "/";

            // creation of output file folders (the suffix subdirectory)
            metric_folder_path = metric_folder_path + std::to_string(suffix_plot) + "/";

            bool is_created = create_folder(metric_folder_path);

            if (!is_created)
            {
              agora::warning("Unable to create the folder \"", metric_folder_path, "\" with errno=", errno);
              throw std::runtime_error("Unable to create the folder \"" + metric_folder_path + "\" with errno=" + std::to_string(errno) );
            }

            // copy the training lines in the output files for the next iteration, with naming siffix++
            // prepare the next files:
            std::fstream current_metric_before_file;
            std::fstream current_metric_after_file;
            std::string file_path_before = metric_folder_path + "before_change_residuals_" + i.first + ".txt";
            std::string file_path_after = metric_folder_path + "after_change_residuals_" + i.first + ".txt";
            current_metric_before_file.open(file_path_before, std::fstream::out);
            current_metric_after_file.open(file_path_after, std::fstream::out);

            if (!current_metric_before_file.is_open())
            {
              agora::warning(log_prefix, "Error: the residuals before the change file has not been created!");
              throw std::runtime_error("Error: the residuals before the change file has not been created!");
            }

            if (!current_metric_after_file.is_open())
            {
              agora::warning(log_prefix, "Error: the residuals after the change file has not been created!");
              throw std::runtime_error("Error: the residuals after the change file has not been created!");
            }

            for (auto& k : j.second.before_change)
            {
              current_metric_before_file << k << std::endl;
            }

            for (auto& k : j.second.after_change)
            {
              current_metric_after_file << k << std::endl;
            }

            current_metric_before_file.flush();
            current_metric_before_file.close();
            current_metric_after_file.flush();
            current_metric_after_file.close();
          }
        }

        // Execute on the specific client the 2nd step of CDT: hypothesis TEST
        confirmed_change = HypTest::perform_hypothesis_test(client_residuals_map, description.application_name, i.first);
        agora::debug(log_prefix, "Client: ", i.first, ": Outcome of hypothesis test: ", confirmed_change);
      }


      if (confirmed_change)
      {
        // as soon as the change is confirmed by any of the metric then exit to classify the current
        // client under analysis as a bad one and move to the next client (if any left)
        agora::debug(log_prefix, "Client: ", i.first, ": Breaking out of the while since the hypothesis test has confirmed the change!");
        break;
      }
      else
      {
        // remove the metrics present in the client_residuals_map from the metric_to_be_analyzed
        // in this way we only leave in the metric_to_be_analyzed just the names of the metrics to yet be analized.
        for (auto& it : client_residuals_map)
        {
          metric_to_be_analyzed.erase(it.first);
        }

        agora::debug(log_prefix, "Client: ", i.first, ": Updated the metrics to still be analyzed. There are still ", metric_to_be_analyzed.size(), " metrics left.");

        // if the metric_to_be_analyzed is empty then it won't enter the next while cycle

        if (metric_to_be_analyzed.size() > 0)
          // if we arrive here it means that metric_to_be_analyzed is not empty, meaning that some metrics to still
          // be analized were not ready. We need to wait for more observations to come hopefully.
        {
          if (timeout <= 0)
          {
            agora::debug(log_prefix, "Client: ", i.first, ": Even though there are some metrics to still be analyzed we run out of time. We consider this client a \"bad\" one...");
            // if we arrive here it means that the 2nd level test has not confirmed the change as of now
            // and we run out of time, we need to move on.
            // We set the current client as a non-valid one then.
            valid_client = false;
            break;
          }
          else
          {
            // we can wait for some more observations to come
            // I do not check (on purpose) if the current total wait time as of now plus the next wait period will be out of time.
            agora::debug(log_prefix, "Client: ", i.first, ": Waiting ", Parameters_beholder::frequency_check, " seconds for some more observations to come hopefully. Current timeout in seconds: ", timeout);
            std::this_thread::sleep_for(std::chrono::seconds(Parameters_beholder::frequency_check));
            timeout -= Parameters_beholder::frequency_check;
          }
        }
      }

      while_counter++;
    }

    agora::debug(log_prefix, "Client: ", i.first, ": Exited from the while cycle.");

    if (valid_client)
    {
      // according to the quality (GOOD=no_change/BAD=confirmed_change) of the currently analyzed client, enqueue it in the good/bad_clients_list
      if (confirmed_change)
      {
        bad_clients_list.emplace(i.first);
        agora::debug(log_prefix, "Emplaced the client ", i.first, " to the list of the bad ones since it confirmed the change.");

      }
      else
      {
        good_clients_list.emplace(i.first);
        agora::debug(log_prefix, "Emplaced the client ", i.first, " to the list of the good ones since it denied the change.");
      }
    }
    else
    {
      timeout_clients_list.emplace(i.first);
      agora::debug(log_prefix, "Emplaced the client ", i.first, " to the list of the timed-out ones since it run-out of time for the hypothesis test.");
    }
  }

  agora::pedantic(log_prefix, "Finished cycling over all the clients for the 2nd step of hierarchical CDT. Computing the fate of the application...");

  // compute the percentage of bad clients and compare it wrt the predefined threshold
  float bad_clients_percentage = ((((float)bad_clients_list.size() + (float)timeout_clients_list.size()) / (float)clients_list_snapshot.size()) * 100);

  // Once I know what the outcome of the hypothesis test is, re-acquire the lock to ensure a consistent global state
  std::unique_lock<std::mutex> guard(mutex);

  // If the 2nd level analysis was not possible at all for lack of data (despite the wait period)
  if (timeout_clients_list.size() == clients_list_snapshot.size())
  {
    // TODO: do Something else??
    agora::info(log_prefix, "WARNING: all the clients run out of time for the hypothesis test. The re-training will be triggered since 100% of the clients are behaving awkwardly.");
  }

  // if the number % of bad clients is over the user-set threshold then trigger the re-training
  // This also covers the case in which all the clients behave badly (100%)
  if (bad_clients_percentage > Parameters_beholder::bad_clients_threshold)
  {
    agora::info(log_prefix, "TRIGGERING RE-TRAINING since the percentage of bad clients [", bad_clients_percentage, "] is greater than the user-selected acceptable one [",
                Parameters_beholder::bad_clients_threshold, "].");

    // In this case the change detected by the 1st level cdt was confirmed by the hypothesis test

    // reset the whole application handler
    clients_blacklist.clear();
    residuals_map.clear();
    ici_cdt_map.clear();
    clients_list.clear();
    clients_list_snapshot.clear();

    // deal with the output files (if enabled)
    if (Parameters_beholder::output_files)
    {
      // for every possible metric available (beholder-enabled metric)
      for (auto& i : reference_metric_names)
      {
        auto search = output_files_map.find(i);

        // if there is the file structure related to that metric
        if (search != output_files_map.end())
        {
          // if the observation file is open then close it
          if (search->second.observations.is_open())
          {
            search->second.observations.close();
          }

          // if the ici file is open then close it
          if (search->second.ici.is_open())
          {
            search->second.ici.close();
          }
        }
      }

      // destroy the current mapping to output files since the next run will have different naming suffixes
      output_files_map.clear();
      // increase the naming suffix counter
      suffix_plot++;
    }

    // check the actual status of the handler
    if (status == ApplicationStatus::DISABLED)
    {
      // if the handler is currently disabled then we need to wait for it to be re-enabled
      previous_status = ApplicationStatus::RETRAINING;
      agora::info(log_prefix, "The re-training has to be triggered, but currently the handler is disabled, so we postpone the trigger to when the handler is re-enabled.");
    }
    else
    {
      agora::info(log_prefix, "Resetting the whole application handler after having triggered the re-training!");
      retraining();
    }

    current_test_observations_counter = 0;
    agora::info(log_prefix, "Handler put-ON-HOLD after having triggered the re-training! Let's wait for a new model...");

  }
  else
  {
    // this is the case in which either there are some bad clients but they are below threshold,
    // or there are no bad clients at all. The behavior is the same.
    // The change is discarded (but so are also the values coming from these bad_clients, if any).
    agora::info(log_prefix, "The hypothesis test REJECTED the change. We keep the current model since the percentage of bad clients [", bad_clients_percentage,
                "] is lower than the user-selected acceptable one [", Parameters_beholder::bad_clients_threshold, "].");

    // if there are some bad clients then put them in the blacklist
    if (bad_clients_list.size() > 0)
    {
      for (auto& i : bad_clients_list)
      {
        agora::debug(log_prefix, "Putting in blacklist client: ", i);
        clients_blacklist.emplace(i);
      }
    }

    // reset the 1st level cdt data structure for all the metrics which have a structure
    // just keep the test configuration settings (training) and reset the rest.
    agora::info(log_prefix, "Resetting the first level of the CDT to its initial configuration and setting back the status to READY.");

    // for every CDT data structure for this application (meaning for every metric that was analyzed in the ICI CDT)
    for (auto& i : ici_cdt_map)
    {
      // reset its ici cdt to just the training configuration
      // restore the sequence number to the number of windows used for training
      i.second.window_number = Parameters_beholder::training_windows;
      // reset the sample mean stuff
      // restore the reference (training) sample mean-mean
      i.second.current_sample_mean_mean = i.second.reference_sample_mean_mean;
      // restore the training CI for the mean
      i.second.current_mean_conf_interval_lower = i.second.reference_mean_conf_interval_lower;
      i.second.current_mean_conf_interval_upper = i.second.reference_mean_conf_interval_upper;

      // if the variance is enabled then do the same for variance too
      if (!Parameters_beholder::variance_off)
      {
        // restore the reference (training) sample variance-mean
        i.second.current_sample_variance_mean = i.second.reference_sample_variance_mean;
        // restore the training CI for the variance
        i.second.current_variance_conf_interval_lower = i.second.reference_variance_conf_interval_lower;
        i.second.current_variance_conf_interval_upper = i.second.reference_variance_conf_interval_upper;
      }
    }

    change_metric_name = "";
    change_window_timestamps = {};

    // deal with the output files
    if (Parameters_beholder::output_files)
    {
      // for every possible metric available (beholder-enabled metric)
      for (auto& i : reference_metric_names)
      {
        auto search = output_files_map.find(i);

        // if there is the file structure related to that metric
        if (search != output_files_map.end())
        {
          // if the file is open
          if (search->second.observations.is_open())
          {
            // the metric subdirectory should have already been created (because we reach this point only if the
            // structure of the metric in analysis has been created), but there should be theoretically
            // no harm in making sure about that.
            std::string metric_folder_path = application_workspace + search->first + "/";

            // creation of output file folders (the suffix subdirectory)
            metric_folder_path = metric_folder_path + std::to_string(suffix_plot + 1) + "/";

            bool is_created = create_folder(metric_folder_path);

            if (!is_created)
            {
              agora::warning("Unable to create the folder \"", metric_folder_path, "\" with errno=", errno);
              throw std::runtime_error("Unable to create the folder \"" + metric_folder_path + "\" with errno=" + std::to_string(errno) );
            }

            // copy the training lines in the output files for the next iteration, with naming siffix++
            // prepare the next files:
            std::fstream current_metric_observations_file;
            std::fstream current_metric_ici_file;
            std::string file_path_obs = metric_folder_path + "observations_" + search->first + "_" + std::to_string(suffix_plot + 1) + ".txt";
            std::string file_path_ici = metric_folder_path + "ici_" + search->first + "_" + std::to_string(suffix_plot + 1) + ".txt";
            current_metric_observations_file.open(file_path_obs, std::fstream::in | std::fstream::out | std::fstream::trunc);
            current_metric_ici_file.open(file_path_ici, std::fstream::in | std::fstream::out | std::fstream::trunc);

            if (!current_metric_observations_file.is_open())
            {
              agora::warning(log_prefix, "Error: the (future) output observation file has not been created!");
              throw std::runtime_error("Error: the (future) output observation file cannot be created!");
            }

            if (!current_metric_ici_file.is_open())
            {
              agora::warning(log_prefix, "Error: the (future) output ici file has not been created!");
              throw std::runtime_error("Error: the (future) output ici file cannot be created!");
            }

            // rewind the two output files to their beginning
            search->second.observations.seekg(0);
            search->second.ici.seekg(0);

            std::string temp_line;

            // copy into the new file the observations related to the training phase
            for (int index = 0; index < Parameters_beholder::window_size * Parameters_beholder::training_windows; index++)
            {
              std::getline(search->second.observations, temp_line);
              current_metric_observations_file << temp_line << std::endl;
            }

            current_metric_observations_file.flush();
            // copy just the first line (training CI info) from the old ici output file to the new one
            std::getline(search->second.ici, temp_line);
            current_metric_ici_file << temp_line << std::endl;
            current_metric_observations_file.flush();
            // close the old files
            search->second.observations.close();
            search->second.ici.close();
            // Replace in the map the struct related to the old files with the one related to the new files.
            // I need to use the move operator to assign because the fstreams are not copyable.
            output_files temp_struct_files = {std::move(current_metric_observations_file), std::move(current_metric_ici_file)};
            search->second = std::move(temp_struct_files);
          }
        }
      }

      // increase the naming suffix counter
      suffix_plot++;
    }

    ici_reset_counter++;
    // resetting the counter of the observations received for the current ici test to just training phase observations
    current_test_observations_counter = Parameters_beholder::window_size * Parameters_beholder::training_windows;

    // check the actual status of the handler
    if (status == ApplicationStatus::DISABLED)
    {
      // if the handler is currently disabled then we need to wait for it to be re-enabled
      previous_status = ApplicationStatus::READY;
      agora::pedantic(log_prefix, "The handler has to be set to READY, but it is currently disabled, so we postpone this to when the handler will be un-paused.");
    }
    else
    {
      // set the status back to ready
      status = ApplicationStatus::READY;
      agora::pedantic(log_prefix, "Setting handler back to READY!");
    }
  }
}


/**
 * @details
 * Method which receives as parameters a row from the list of observations from a specific client from the trace
 * and parses that and inserts its residuals in the corresponding hashmap.
 * It also deals with the separation of residuals belonging to the before-the-change period
 * wrt those belonging to the after-the-change period.
 */
void RemoteApplicationHandler::parse_and_insert_observations_for_client_from_trace(std::unordered_map<std::string, residuals_from_trace>& client_residuals_map,
    const observation_t j, const std::set<std::string>& metric_to_be_analyzed)
{
  agora::debug("\n", log_prefix, "String from trace to be parsed: ", j);
  // parse the string. Taking into account the number of enabled metrics in the current observation.
  // We need to know which observed metrics we have to retrieve and compare them with the model estimation.
  // We only keep production phase observations, and of these we only consider metrics (enabled for the beholder)
  // for which the real observed values is available, to be compared with the model value.
  std::string obs_client_id;
  timestamp_fields timestamp;
  std::vector <std::string> obs_configuration;
  std::vector <std::string> obs_features;
  std::vector <std::string> obs_metrics;
  std::vector <std::string> obs_estimates;

  std::vector<std::string> metric_fields_vec;
  std::stringstream str_observation(j);

  str_observation >> timestamp.seconds;
  str_observation >> timestamp.nanoseconds;

  str_observation >> obs_client_id;
  //agora::debug(log_prefix, "Client_id parsed: ", obs_client_id);

  int num_knobs = description.knobs.size();

  while ( num_knobs > 0 )
  {
    std::string current_knob;
    str_observation >> current_knob;
    obs_configuration.emplace_back(current_knob);
    //agora::debug(log_prefix, "Client: ", obs_client_id, ": Knob parsed: ", current_knob);
    num_knobs--;
  }

  int num_features = description.features.size();

  while ( num_features > 0 )
  {
    std::string current_feature;
    str_observation >> current_feature;
    obs_features.emplace_back(current_feature);
    //agora::debug(log_prefix, "Client: ", obs_client_id, ": Feature parsed: ", current_feature);
    num_features--;
  }

  int num_metrics = metric_to_be_analyzed.size();

  while ( num_metrics > 0 )
  {
    std::string current_metric;
    str_observation >> current_metric;
    obs_metrics.emplace_back(current_metric);
    //agora::debug(log_prefix, "Client: ", obs_client_id, ": Metrics parsed: ", current_metric);
    num_metrics--;
  }

  num_metrics = metric_to_be_analyzed.size();

  // variable to understand if the current row from trace was from a training phase
  // It would have ALL the estimates to "N/A".
  // In that case the current row in analysis can be discarded because there is no way
  // of computing the residuals.
  // Theoretically by design we should just not receive the training lines from the trace at all,
  // (because we filter the trace from the time the beholder has first seen the client),
  // this is just and additional check.
  bool is_training_row = true;

  while ( num_metrics > 0 )
  {
    std::string current_estimate;
    str_observation >> current_estimate;

    // if there is at least an estimate different from "N/A" then the current row is not from
    // training and can be used in the CDT analysis for the residuals.
    if ((current_estimate != "N/A") || (!is_training_row))
    {
      is_training_row = false;
    }

    obs_estimates.emplace_back(current_estimate);
    //agora::debug(log_prefix, "Client: ", obs_client_id, ": Estimate parsed: ", current_estimate);
    num_metrics--;
  }

  // Check if we have consistency in the quantity of measures parsed,
  // i.e. if the vector of metric names is as big as the one of the observed metrics
  // and is the same size as the number of metrics enabled for the beholder for the current application
  if ((obs_metrics.size() != obs_estimates.size()) || (obs_metrics.size() != metric_to_be_analyzed.size()))
  {
    agora::warning(log_prefix, "Client: ", obs_client_id, ": Error in the parsed observation, mismatch in the number of fields.");
    return;
  }

  // if the current row from trace is from a training phase discard this row and go to the next;
  if (is_training_row)
  {
    agora::debug(log_prefix, "Client: ", obs_client_id, ": Discarding current row because it was from a training phase");
    return;
  }

  agora::debug(log_prefix, "Client: ", obs_client_id, ": Done with the parsing of the current row from trace, starting the validation and insertion of the metrics into the residual buffers.");


  // Validate the observation and Insert the residuals in the right residual maps structure
  auto name_ref = metric_to_be_analyzed.begin();

  bool skip_change_window = false;
  bool before_change_window = false;
  bool after_change_window = false;

  // for every metric in the row:
  for (auto index = 0; index < metric_to_be_analyzed.size(); index++, std::advance(name_ref, 1))
  {
    // Check whether the metric in analysis was available and valid or if it was a disabled one-->"null" in trace-->parsed as "N/A"
    // The idea is that if the current metric in analysis is disabled of course we do not insert this metric (for this row)
    // in the buffers. But in order to be disabled there has to be (by design) a "N/A" in both the observation and the estimate for
    // that metric. If that is not the case and just one of the two is "N/A" then there is an error somewhere, because it is
    // theoretically impossible.
    if (obs_estimates[index] == "N/A")
    {
      // for the current version, if a metric is disabled its corresponding prediction must be disable too
      if (obs_metrics[index] == "N/A")
      {
        // skip the comparison on this metric between it was not enabled
        return;
      }
      else
      {
        agora::warning(log_prefix, "Client: ", obs_client_id, ": Error in the parsed observation, mismatch between the observed (!N/A) and predicted (N/A) metric.");
        return;
      }
    }
    // to catch the case in which the metric was null but the estimate was present (not null). Theoretically impossible by design.
    else if (obs_metrics[index] == "N/A")
    {
      agora::warning(log_prefix, "Client: ", obs_client_id, ": Error in the parsed observation, mismatch between the observed (N/A) and predicted (!N/A) metric.");
      return;
    }

    // if we arrive here then the parsed metric should be valid (one of the enabled ones at least)

    // NB: note that the residual is computed with abs()!!
    float current_residual = fabs(std::stof(obs_estimates[index]) - std::stof(obs_metrics[index]));
    agora::debug(log_prefix, "Client: ", obs_client_id, ": Current residual for metric ", *name_ref, " is: ", current_residual);

    auto search = client_residuals_map.find(*name_ref);

    // Here we need to save the element before the change window in a vector (1st in struct: "before_change"),
    // and the elements after that window in another corresponding vector (2nd in struct: "after_change").
    // NB: the timestamps of the hypothetical change window used here are referred obviously
    // to the metric which first detected the change. It may be the same metric under analysis here
    // or not.
    if (search != client_residuals_map.end())
    {
      agora::debug(log_prefix, "Client: ", obs_client_id, ": metric ", *name_ref, " already present, filling buffer");

      // metric already present, need to add to the buffer the new residual
      // compare timestamp: if before the change insert in the 1st vector of the struct
      if (std::stol(timestamp.seconds) < std::stol(change_window_timestamps.front.seconds))
      {
        // if the current seconds_from_epoch is older than the one from the first element of the
        // change window then add it to the "before" change window vector.
        search->second.before_change.emplace_back(current_residual);
        before_change_window = true;
      }
      // if the change window is contained in the same seconds_from_epoch (seconds of front and back are the same)
      else if ((std::stol(change_window_timestamps.front.seconds) == std::stol(change_window_timestamps.back.seconds)) && (std::stol(timestamp.seconds) == std::stol(change_window_timestamps.front.seconds)))
      {
        // if the current observation seconds is the same as the seconds of the 1st element of the
        // change window then compare the nanoseconds. If the current observation's timestamp
        // comes first in the day then add it to the "before" change window vector
        if (std::stol(timestamp.nanoseconds) < std::stol(change_window_timestamps.front.nanoseconds))
        {
          search->second.before_change.emplace_back(current_residual);
          before_change_window = true;
        }
        // if the current observation seconds is the same as the seconds of the last element of the
        // change window then compare the time. If the current observation's timestamp
        // comes later in the day then add it to the "after" change window vector
        else if (std::stol(timestamp.nanoseconds) > std::stol(change_window_timestamps.back.nanoseconds))
        {
          search->second.after_change.emplace_back(current_residual);
          after_change_window = true;
        }
        else
        {
          skip_change_window = true;
        }
      }
      // if the change window is not contained in the same seconds (seconds of front and back are not the same)
      else if (std::stol(timestamp.seconds) == std::stol(change_window_timestamps.front.seconds))
      {
        // if the current observation seconds is the same as the seconds of the 1st element of the
        // change window then compare the time. If the current observation's timestamp
        // comes first in the day then add it to the "before" change window vector
        if (std::stol(timestamp.nanoseconds) < std::stol(change_window_timestamps.front.nanoseconds))
        {
          search->second.before_change.emplace_back(current_residual);
          before_change_window = true;
        }
        else
        {
          skip_change_window = true;
        }

        // if after the change insert in the 2nd vector of the struct
      }
      else if (std::stol(timestamp.seconds) > std::stol(change_window_timestamps.back.seconds))
      {
        // if the current seconds is newer than the one from the last element of the
        // change window then add it to the "after" change window vector.
        search->second.after_change.emplace_back(current_residual);
        after_change_window = true;
      }
      else if (std::stol(timestamp.seconds) == std::stol(change_window_timestamps.back.seconds))
      {
        // if the current observation seconds is the same as the seconds of the last element of the
        // change window then compare the time. If the current observation's timestamp
        // comes later in the day then add it to the "after" change window vector
        if (std::stol(timestamp.nanoseconds) > std::stol(change_window_timestamps.back.nanoseconds))
        {
          search->second.after_change.emplace_back(current_residual);
          after_change_window = true;
        }
        else
        {
          skip_change_window = true;
        }
      }
      else
      {
        skip_change_window = true;
      }

    }
    else
    {
      // if we did not find any key with the current metric's name under analysis in the struct
      // then theoretically we should initialize the corresponding struct for the metric.
      // Check that this first value is actually before the change window. If that is the case then
      // insert the current observation in the vector of the residuals before the change.
      // Theoretically one would expect that the first observation for a certain metric coming
      // from a client would always be before the change window. But actually this is not necessarily
      // true. A client could have started the computation after the change window,
      // and we would have detected the change thanks to observations coming from other
      // clients that were already working before the one under analysis, obviously.
      // Of course we are interested in comparing the behavior of the distribution of the
      // observations before and after the change. So in a situation in which we just have
      // "after" the change observations we immediately skip the analysis for the current metric for this client.

      // bool to control if the first observation is actually before the change window
      bool valid_metric = false;

      if (std::stol(timestamp.seconds) < std::stol(change_window_timestamps.front.seconds))
      {
        // if the current date is older than the one from the first element of the
        // change window then add it to the "before" change window vector.
        valid_metric = true;
      }
      else if (std::stol(timestamp.seconds) == std::stol(change_window_timestamps.front.seconds))
      {
        // if the current observation date is the same as the date of the 1st element of the
        // change window then compare the time. If the current observation's timestamp
        // comes first in the day then add it to the "before" change window vector
        if (std::stol(timestamp.nanoseconds) < std::stol(change_window_timestamps.front.nanoseconds))
        {
          valid_metric = true;
        }
      }

      // if the first observation is before the change window then initialize its struct
      if (valid_metric)
      {
        before_change_window = true;
        agora::debug(log_prefix, "Client: ", obs_client_id, ": creation of buffer for metric and first insertion: ", *name_ref);
        // need to create the mapping for the current metric. It's the first time you meet this metric
        std::vector<float> temp_vector_before;
        std::vector<float> temp_vector_after;
        temp_vector_before.emplace_back(current_residual);
        residuals_from_trace temp_struct = {temp_vector_before, temp_vector_after};
        client_residuals_map.emplace(*name_ref, temp_struct);
      }
      else
      {
        agora::debug(log_prefix, "Client: ", obs_client_id, ": Skipping the creation of buffer for metric ", *name_ref, " because we do not have observations before the hypothetical change window");
      }
    }
  }

  if (before_change_window)
  {
    agora::pedantic(log_prefix, "Client: ", obs_client_id, ": successfully parsed and inserted into the before-change-window buffer the observation: ", j);
  }
  else if (after_change_window)
  {
    agora::pedantic(log_prefix, "Client: ", obs_client_id, ": successfully parsed and inserted into the after-change-window buffer the observation: ", j);
  }
  else if (skip_change_window)
  {
    agora::pedantic(log_prefix, "Client: ", obs_client_id, ": successfully parsed but not inserted into buffers (because belonging to the selected change window) the observation: ", j);
  }
  else
  {
    agora::pedantic(log_prefix, "Client: ", obs_client_id, ": successfully parsed the observation: ", j);
  }
}
