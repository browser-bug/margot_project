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


#include "beholder/application_handler_beholder.hpp"
#include "beholder/parameters_beholder.hpp"
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
  //agora::debug(log_prefix, "Number of total metrics: ", description.metrics.size());
  //agora::debug(log_prefix, "Window size: ", Parameters_beholder::window_size);
  //agora::debug(log_prefix, "Number of windows used for training: ", Parameters_beholder::training_windows);


  // create the workspace root folder
  application_workspace = Parameters_beholder::workspace_folder;

  if (Parameters_beholder::workspace_folder.back() != '/')
  {
    application_workspace.append("/");
  }

  application_workspace.append("beholder/");

  std::stringstream path_stream(description.application_name);
  std::string&& current_path = "";

  while (std::getline(path_stream, current_path, '/'))
  {
    application_workspace.append(current_path + "/");
    const bool is_created = create_folder(application_workspace);

    if (!is_created)
    {
      agora::warning("Unable to create the folder \"", application_workspace, "\" with errno=", errno);
      throw std::runtime_error("Unable to create the folder \"" + application_workspace + "\" with errno=" + std::to_string(errno) );
    }
  }

  // write the output file used to plot the training/operational phase windows
  std::ofstream outfileConf (application_workspace + "configTestWindows.txt", std::ofstream::out);
  outfileConf << Parameters_beholder::window_size << " ";
  outfileConf << Parameters_beholder::training_windows << " ";
  outfileConf.close();
}

void RemoteApplicationHandler::new_observation( const std::string& values )
{

  // lock the mutex to ensure a consistent global state
  std::unique_lock<std::mutex> guard(mutex);

  // check whether we can analyze the incoming payload or if we need to discard it
  // according to the handler status: ready/computing
  if ( status == ApplicationStatus::COMPUTING )
  {
    return;
  }

  // release the lock while parsing the message
  // QUESTION: should I release the lock here? Could it break the order of messages and build a not consistent window?

  // struct to store the current observation received
  Observation_data observation;

  // parse the received observation
  int parse_outcome = parse_observation(observation, values);

  if (parse_outcome == 1)
  {
    return;
  }

  // fill in the buffers with the current observations
  int filling_outcome = fill_buffers(observation);

  if (filling_outcome == 1)
  {
    return;
  }

  agora::pedantic(log_prefix, "Observation successfully parsed and inserted in buffers.");

  // just universal variables to control the change detection and thus the flow of the CDT itself
  // since we do not care to actually check all the metrics, but it is enough a metric (the first)
  // to trigger the 2nd step of the hierarchiacla CDT.
  bool change_detected = false;
  std::string change_metric;

  // Check whether one (or more) buffers is (are) filled in
  // up to the beholder's window_size parameter.
  for (auto& i : residuals_map)
  {
    // if a change has been detected then exit from the for cycle to stop the 1st step of the CDT.
    // Before doing so reset all the buffers for the metrics, whatever their state is.
    if (change_detected)
    {
      agora::debug(log_prefix, "Resetting buffer for metric ", i.first, " after change detected in metric: ", change_metric);
      i.second.clear();
      // go to the next metric (next iteration of the for cycle), if available, until the cycle has finished
      continue;
    }

    agora::debug(log_prefix, "Current filling level of the buffer for metric ", i.first, ": ", i.second.size());

    // if any buffer is filled in then perform the ici cdt on it
    if (i.second.size() == Parameters_beholder::window_size)
    {
      agora::debug(log_prefix, "Buffer for metric ", i.first, " filled in, starting CDT on the current window.");

      // Check if we already have a ici_cdt_map for the current metric, if not create it
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
        agora::debug(log_prefix, "CDT data for metric ", i.first, " NOT found. Initialized CDT data structure and starting CDT!");
        change_detected = IciCdt::perform_ici_cdt(new_ici_struct, i.second, output_files_map);
        ici_cdt_map.emplace(i.first, new_ici_struct);
        // It cannot detect a change if this is the first full window for that metric, we are in training of cdt
        // so basically avoid the if to check for the boolean "change_detected"
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
        change_metric = i.first;
        // set the status variable according so that the lock can be released
        status = ApplicationStatus::COMPUTING;
        // Empty the (filled-in) buffer for the current metric.
        i.second.clear();
        // every time a new change is detected by the 1st level test we reset the blacklist of clients
        // In this way we check whether they are still behaving badly with the 2nd level test,
        // and we give them the chance to possibly be re-considered as good clients
        // from which to collect observations also for the 1st level test in the future.
        clients_blacklist.clear();
      }
    }
  }

  // We only perform the 2nd step of the CDT only when the 1st level detects a change.
  // If the change is not detected then the 2nd step is not performed and the method returns
  // and will keep performing the 1st level cdt with the future new observations.


  // STEP 2 of CDT: analysis with granularity on the single client

  // if need to start the step 2 of CDT:
  if (status == ApplicationStatus::COMPUTING)
  {
    guard.unlock();

    agora::pedantic(log_prefix, "Entering 2nd level of CDT! Getting the list of clients running the application...");

    // to store the observations belonging to a pair application-client_name_t
    observations_list_t observations_list;

    // store the list of clients working on the application_name
    application_list_t clients_list;

    // store the list of good-enough clients
    application_list_t good_clients_list;

    // store the list of bad clients for the current observation (not yet blacklisted)
    application_list_t bad_clients_list;

    // clients which did not answer in time because of the timeout
    application_list_t timeout_clients_list;

    // query to retrieve all the distinct clients which are running a specif application
    // to be performed in case of positive CDT, for the second level hypothesis test.
    // It is better to always re-run this query and avoid saving the client list for re-use
    // because there could be some new clients.
    clients_list = agora::io::storage.load_clients(description.application_name);

    // Check that the list of clients is not is not empty
    if (clients_list.size() == 0)
    {
      agora::info(log_prefix, "Something is wrong, the list of clients received from the DB is empty!");
      return;
    }

    agora::debug(log_prefix, "Client list without duplicates:");

    for (auto& i : clients_list)
    {
      agora::debug(log_prefix, i);
    }

    // initialize timeout shared (consumed) among all the clients
    int timeout = Parameters_beholder::timeout;



    // cycle over all the clients of the specific application
    for (auto& i : clients_list)
    {
      agora::pedantic(log_prefix, "Starting the 2nd level of CDT for client: ", i, ". Setting up the metrics to be analyzed...");

      // initialized as a duplicate of reference_metric_names, that is to say all the metrics enabled for the
      // beholder analysis this list contains the metrics yet to be tested with the hypothesis test, so once a
      // metric has been analyzed it will be removed from this structure
      std::set<std::string> metric_to_be_analyzed;

      for (auto& j : reference_metric_names)
      {
        metric_to_be_analyzed.emplace(j);
        agora::debug(log_prefix, "Metric to be analyzed for client ", i, ": ", j);
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
        agora::debug(log_prefix, "Entering while cycle for the ", while_counter, " time for client ", i, " with still ", metric_to_be_analyzed.size(), " metrics to be analyzed out of a total of ",
                     reference_metric_names.size(), " beholder-enabled metrics.");
        // data structure to save the residuals for the specific application-client pair before the change
        // the structure is organized as a map which maps the name of the metric to a pair
        // The first element of the pair is a vector containing the residuals for that metric before the change windows
        // The second element of the pair is a vector containing the residuals for that metric after the change windows
        std::unordered_map<std::string, std::pair < std::vector<float>, std::vector<float>>> client_residuals_map;

        // preparing the 'select' query for Cassandra for the 2nd step of cassandra
        // It is done here on the basis of whixch metrics are left to be analyzed
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

        agora::debug(log_prefix, "Querying DB for client ", i, " to get its observations from the trace table.");
        agora::debug(log_prefix, "Query: ", query_select);
        //observations_list = agora::io::storage.load_client_observations(description.application_name, "alberto_Surface_Pro_2_6205", query_select);
        observations_list = agora::io::storage.load_client_observations(description.application_name, i, query_select);

        agora::debug("\n", log_prefix, "Trace query executed successfully for client ", i, ", starting the parsing of its observations.");

        // cycle over each row j of the trace for each client i (for the current application)
        for (auto& j : observations_list)
        {
          parse_and_insert_observations_for_client_from_trace(client_residuals_map, j, metric_to_be_analyzed);
        }

        agora::debug(log_prefix, "Finished parsing and putting in the respective buffers the result of the trace query for client: ", i);
        agora::debug(log_prefix, "Computing which metrics have enough observations to actually perform the hypothesis test for client: ", i);

        // let's analyze if any of the metrics has enough observations to perform the test
        // remove the metrics which cannot be analyzed from the client_residuals_map
        for (auto it = client_residuals_map.begin(); it != client_residuals_map.end();)
        {
          if (it->second.first.size() < Parameters_beholder::min_observations || it->second.second.size() < Parameters_beholder::min_observations)
          {
            it = client_residuals_map.erase(it);
          }
          else
          {
            ++it;
          }
        }

        if (client_residuals_map.size() == metric_to_be_analyzed.size())
        {
          agora::debug(log_prefix, "All the ", client_residuals_map.size(), " out of ", metric_to_be_analyzed.size(), " metrics to still be analyzed can perform the test for client ", i);
        }
        else
        {
          agora::debug(log_prefix, "Just ", client_residuals_map.size(), " out of ", metric_to_be_analyzed.size(), " metrics to still be analyzed can perform the test for client ", i);
        }

        // if there is at least a metric on which we can perform the hypothesis test
        if (client_residuals_map.size() > 0)
        {
          // Execute on the specific client the 2nd step of CDT: hypothesis TEST
          confirmed_change = HypTest::perform_hypothesis_test(client_residuals_map, description.application_name, i);
        }

        agora::debug(log_prefix, "Outcome of hypothesis test for client ", i, ": ", confirmed_change);

        if (confirmed_change)
        {
          // as soon as the change is confirmed by any of the metric then exit to classify the current
          // client under analysis as a bad one and move to the next client (if any left)
          agora::debug(log_prefix, "Breaking out of the while since the hypothesis test on client ", i, " has confirmed the change!");
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

          agora::debug(log_prefix, "Updated the metrics to still be analyzed for client ", i, ". There are still ", metric_to_be_analyzed.size(), " metrics left.");

          // if the metric_to_be_analyzed is empty then it won't enter the next while cycle

          if (metric_to_be_analyzed.size() > 0)
            // if we arrive here it means that metric_to_be_analyzed is not empty, meaning that some metrics to still
            // be analized were not ready. We need to wait for more observations to come hopefully.
          {
            if (timeout <= 0)
            {
              agora::debug(log_prefix, "Even though there are some metrics to still be analyzed for client ", i, " we run out of time. We consider this client a \"bad\" one...");
              // if we arrive here it means that the 2nd level test has not confirmed the change as of now
              // and we run out of time, we need to move on.
              // We set the current client as a non-valid one then.
              valid_client = false;
              break;
            }
            else
            {
              // we can wait for some more observations to come
              // I chose to wait even if the current timeout-waitperiod is theoretically out_of_time already
              agora::debug(log_prefix, "Waiting ", Parameters_beholder::frequency_check, " seconds for some more observations to come hopefully. Current timeout in seconds: ", Parameters_beholder::timeout);
              std::this_thread::sleep_for(std::chrono::milliseconds(Parameters_beholder::frequency_check));
              timeout -= Parameters_beholder::frequency_check;
            }
          }
        }
      }

      agora::debug(log_prefix, "Exited from the while cycle for client: ", i);

      if (valid_client)
      {
        // according to the quality (GOOD=no_change/BAD=confirmed_change) of the currently analyzed client, enqueue it in the good/bad_clients_list
        if (confirmed_change)
        {
          bad_clients_list.emplace(i);
          agora::debug(log_prefix, "Emplaced the client ", i, " to the list of the bad ones since it confirmed the change.");

        }
        else
        {
          good_clients_list.emplace(i);
          agora::debug(log_prefix, "Emplaced the client ", i, " to the list of the good ones since it denied the change.");
        }
      }
      else
      {
        timeout_clients_list.emplace(i);
        agora::debug(log_prefix, "Emplaced the client ", i, " to the list of the timed-out ones since it run-out of time for the hypothesis test.");
      }
    }

    agora::pedantic(log_prefix, "Finished cycling over all the clients for the 2nd step of hierarchical CDT. Computing the fate of the application...");


    // compute the percentage of bad clients and compare it wrt the predefined threshold
    float bad_clients_percentage = (((bad_clients_list.size() + timeout_clients_list.size()) / clients_list.size()) * 100);

    // Once I know what the outcome of the hypothesis TEST is, re-acquire the lock
    guard.lock();

    if (timeout_clients_list.size() == 0)
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

      // Basically in this case the change detected by the 1st level cdt was confirmed by the hypothesys test

      // need to trigger RE-training
      // this automatically deals with the deletion of the model and of the trace and with the reset of the doe
      // in this version the trace is deleted just from the top element in the table up to the last element of the training window
      if (Parameters_beholder::no_trace_drop)
      {
        auto search_ici_map = ici_cdt_map.find(change_metric);

        if (search_ici_map != ici_cdt_map.end())
        {
          agora::pedantic(log_prefix, "Deleting the model, restoring the DOE, deleting just the rows of the trace which are before the detected change window.");
          send_agora_command("retraining " + std::to_string(search_ici_map->second.back_year_month_day) + "," + std::to_string(search_ici_map->second.back_time_of_day));
        }
      }
      else
      {
        agora::pedantic(log_prefix, "Deleting the model, restoring the DOE, deleting the whole trace.");
        // delete the whole trace
        send_agora_command("retraining");
      }

      // reset the whole application handler
      clients_blacklist.clear();
      residuals_map.clear();
      ici_cdt_map.clear();

      // for every possible metric available (beholder-enabled metric)
      for (auto& i : reference_metric_names)
      {
        auto search = output_files_map.find(i);

        // if there is the file structure related to that metric
        if (search != output_files_map.end())
        {
          // if the observation file is open then close it
          if (search->second.first.is_open())
          {
            search->second.first.close();
          }

          // if the ici file is open then close it
          if (search->second.second.is_open())
          {
            search->second.second.close();
          }
        }
      }

      // destroy the current mapping to output files since the next run will have different naming suffixes
      output_files_map.clear();
      // increase the naming suffix counter
      suffix_plot++;
      agora::info(log_prefix, "Resetting the whole application handler after having triggered the re-training!");
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

        // if the data structure under analysis belongs to the metric which triggered the change first
        if (i.second.metric_name == change_metric)
        {
          // reset the change window timestamp to be sure
          i.second.front_year_month_day = 0;
          i.second.front_time_of_day = 0;
          i.second.back_year_month_day = 0;
          i.second.back_time_of_day = 0;
        }
      }

      // for every possible metric available (beholder-enabled metric)
      for (auto& i : reference_metric_names)
      {
        auto search = output_files_map.find(i);

        // if there is the file structure related to that metric
        if (search != output_files_map.end())
        {
          // if the file is open
          if (search->second.first.is_open())
          {
            // copy the training lines in the output files for the next iteration, with naming siffix++
            // prepare the next files:
            std::fstream current_metric_observations_file;
            std::fstream current_metric_ici_file;
            std::string file_path_obs = application_workspace + search->first + "/observations_" + search->first + "_" + std::to_string(suffix_plot + 1) + ".txt";
            std::string file_path_ici = application_workspace + search->first + "/ici_" + search->first + "_" + std::to_string(suffix_plot) + ".txt";
            current_metric_observations_file.open(file_path_obs, std::fstream::out);
            current_metric_ici_file.open(file_path_ici, std::fstream::out);
            std::string temp_line;

            // copy into the new file the observations related to the training phase
            for (int index = 0; index < Parameters_beholder::window_size * Parameters_beholder::training_windows; index++)
            {
              std::getline(search->second.first, temp_line);    // Check whether this method automatically rewinds the file and keeps the position across cycles. It should.
              current_metric_observations_file << temp_line << std::endl; // TODO: check whether this result in a double endline
            }

            current_metric_observations_file.flush();
            // copy just the first line (training CI info) from the old ici output file to the new one
            std::getline(search->second.second, temp_line);    // Check whether this method automatically rewinds the file and keeps the position across cycles. It should.
            current_metric_ici_file << temp_line << std::endl; // TODO: check whether this result in a double endline
            current_metric_observations_file.flush();
            // close the old file
            search->second.first.close();
            // remove from the map the old file
            // Or even better replace the old with the new ones, so that you do not need to delete a mapping and
            // re-create it.
            // Basically I need to replace the pair here.
            // I need to use the move operator to assign because the fstreams are not coyable...
            auto temp_pair_file = std::make_pair(std::move(current_metric_observations_file), std::move(current_metric_ici_file));
            search->second = std::move(temp_pair_file);
          }
        }
      }

      // destroy the current mapping to output files since the next run will have different naming suffixes
      output_files_map.clear();
      // increase the naming suffix counter
      suffix_plot++;

      // set the status back to ready
      status = ApplicationStatus::READY;
    }
  }

  // The following command will (probably) be never used.
  // if (false /*need to enable metrics*/)
  // {
  //   send_margot_command("metrics_on");
  // }
  return;
}


// method to parse the received observation from mqtt messages
int RemoteApplicationHandler::parse_observation(Observation_data& observation, const std::string& values)
{
  // declare the temporary variables to store the fields of the incoming message
  std::string metrics;
  std::string metric_fields;
  std::string estimates;
  std::string user_enabled_metrics;

  // parse the timestamp and the client_id
  std::stringstream stream(values);
  stream >> observation.timestamp;
  agora::debug(log_prefix, "Timestamp: ", observation.timestamp);
  stream >> observation.client_id;
  agora::debug(log_prefix, "client_id: ", observation.client_id);

  // check whether the client which sent the current observation is in the blacklist.
  // if that's the case then discard the observation, otherwise keep parameter_string
  auto search = clients_blacklist.find(observation.client_id);

  if (search != clients_blacklist.end())    // if client name found in the blacklist than return
  {
    agora::info(log_prefix, "Observation from client ", observation.client_id, " rejected because blacklisted client");
    return 1;
  }

  // get the observed values
  stream >> metrics;
  agora::debug(log_prefix, "metrics: ", metrics);

  // gets the model values
  stream >> estimates;
  agora::debug(log_prefix, "estimates: ", estimates );

  // gets the name of the fields of the metric to be filled in
  // NB: note that the beholder observation message always contains the metric names, also when all the metrics are enabled.
  // This is because in this way we know which metrics must be observed by the beholder
  // according to the user's settings in the XML configuration file.
  // This is needed because there could be metrics which would be available but that should not be monitored
  // by the beholder, again according to the user preferences.
  // The margot heel just sends us the name of the metrics which should be observed.
  // While gathering the metric names from the DB would have meant to lose the information of which metric
  // should be actually monitored out of all those available.
  stream >> metric_fields;
  agora::debug(log_prefix, "metric_fields: ", metric_fields);

  // if this is the first message then parse also the last part of the message where the heel enqueues the whole list of user-enabled-beholder-metrics
  // if this list is not present than there is the assumption that all the metrics should be analized, even though the user did not explicitely stated that
  if (reference_metric_names.size() == 0)
  {
    stream >> user_enabled_metrics;

    //if there is no list than get the reference metrics from the application description, it is assumed that all of them are enabled
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
      // and put them into the reference set for the user enabled beholder metrics
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
  // build the vector of metric names provided in the observation
  std::stringstream ssmf(metric_fields);

  while ( ssmf.good() )
  {
    std::string substr;
    getline( ssmf, substr, ',' );
    observation.metric_fields_vec.push_back( substr );
  }

  for (auto& i : observation.metric_fields_vec)
  {
    agora::debug(log_prefix, "metric_fields separated: ", i);
  }

  // build the vector of observed metrics provided in the observation
  std::stringstream ssm(metrics);

  while ( ssm.good() )
  {
    std::string substr;
    getline( ssm, substr, ',' );
    observation.metrics_vec.push_back( std::stof(substr) );
  }

  for (auto& i : observation.metrics_vec)
  {
    agora::debug(log_prefix, "metrics separated: ", i);
  }

  // build the vector of observed metrics provided in the observation
  std::stringstream ssme(estimates);

  while ( ssme.good() )
  {
    std::string substr;
    getline( ssme, substr, ',' );
    observation.estimates_vec.push_back( std::stof(substr) );
  }

  for (auto& i : observation.estimates_vec)
  {
    agora::debug(log_prefix, "estimates separated: ", i);
  }


  // Check if we have consistency in the quantity of measures received,
  // i.e. if the vector of metric names is as big as the one of the observed metrics and the one of the estimates
  if ((observation.metric_fields_vec.size() != observation.metrics_vec.size()) || (observation.metrics_vec.size() != observation.estimates_vec.size()))
  {
    agora::warning(log_prefix, "Error in the observation received, mismatch in the number of fields.");
    return 1;
  }

  return 0;
}

// method which adds the parsed values to the current observation to the respective buffers
int RemoteApplicationHandler::fill_buffers(const Observation_data& observation)
{
  // Insert the residuals in the right buffers according to the metric name
  for (auto index = 0; index < observation.metric_fields_vec.size(); index++)
  {
    agora::pedantic(log_prefix, "Started the process of filling in the buffers with the parsed observations.");

    // NB: note that the residual is computed with abs()!!
    auto current_residual = abs(observation.estimates_vec[index] - observation.metrics_vec[index]);
    agora::debug(log_prefix, "Current residual for metric ", observation.metric_fields_vec[index], " is: ", current_residual);

    auto search = residuals_map.find(observation.metric_fields_vec[index]);

    if (search != residuals_map.end())
    {
      agora::debug(log_prefix, "metric ", observation.metric_fields_vec[index], " already present, filling buffer");
      // metric already present, need to add to the buffer the new residual
      auto temp_pair = std::make_pair(current_residual, observation.timestamp);
      search->second.emplace_back(temp_pair);
      // manage the output to file
      auto search_file = output_files_map.find(observation.metric_fields_vec[index]);

      if (search_file == output_files_map.end())
      {
        agora::warning(log_prefix, "Error: attempting to write to a file_output_map which does not exist.");
        return 1;
      }

      // if we arrive here (as it is supposed to be) we need to append  the current observation
      // to an already created output-file mapping for the current metric
      search_file->second.first << current_residual << std::endl;
      search_file->second.first.flush();
    }
    else
    {
      agora::debug(log_prefix, "creation of buffer for metric and first insertion: ", observation.metric_fields_vec[index]);
      // need to create the mapping for the current metric. It's the first time you meet this metric
      auto temp_pair = std::make_pair(current_residual, observation.timestamp);
      std::vector<std::pair <float, std::string>> temp_vector;
      temp_vector.emplace_back(temp_pair);
      residuals_map.emplace(observation.metric_fields_vec[index], temp_vector);
      // manage the output to file
      auto search_file = output_files_map.find(observation.metric_fields_vec[index]);

      if (search_file != output_files_map.end())
      {
        agora::warning(log_prefix, "Error: attempting the creation of a file_output_map which is already present.");
        return 1;
      }

      // if we arrive here (as it is supposed to be) we need to create a new output-file mapping for the current metric
      // current output file
      std::fstream current_metric_observations_file;
      std::fstream current_metric_ici_file;
      //std::vector<std::ofstream> test;
      //test.emplace_back("bobo", std::ofstream::out);
      std::string file_path_obs = application_workspace + observation.metric_fields_vec[index] + "/observations_" + observation.metric_fields_vec[index] + "_" + std::to_string(suffix_plot) + ".txt";
      std::string file_path_ici = application_workspace + observation.metric_fields_vec[index] + "/ici_" + observation.metric_fields_vec[index] + "_" + std::to_string(suffix_plot) + ".txt";
      current_metric_observations_file.open(file_path_obs, std::fstream::out);
      current_metric_ici_file.open(file_path_ici, std::fstream::out);
      current_metric_observations_file << current_residual << std::endl;
      current_metric_observations_file.flush();
      auto temp_pair_file = std::make_pair(std::move(current_metric_observations_file), std::move(current_metric_ici_file));

      //std::pair<std::ofstream, std::ofstream> temp_pair_file = std::make_pair(file_path_obs, std::ofstream::out, file_path_ici, std::ofstream::out);
      //std::pair<std::ofstream, std::ofstream> temp_pair_file (file_path_obs, std::ofstream::out, file_path_ici, std::ofstream::out);
      output_files_map.emplace(observation.metric_fields_vec[index], std::move(temp_pair_file));
    }
  }

  return 0;
}

// method which receives as parameters the list of observations from a specific client from the trace
// and parses them and inserts its residuals in the corresponding map structure
void RemoteApplicationHandler::parse_and_insert_observations_for_client_from_trace(std::unordered_map<std::string, std::pair < std::vector<float>, std::vector<float>>>& client_residuals_map,
    const observation_t j, const std::set<std::string>& metric_to_be_analyzed)
{
  agora::debug("\n", log_prefix, "String from trace to be parsed: ", j);
  // parse the string. Taking into account the number of enabled metrics in the current observation.
  // we need to know which metric(s) we have to retrieve and compare with the model estimation.
  // Basically we keep only production phase observations, and of these we only consider metrics (enabled for the beholder)
  // for which the real observed values is available, to be compared with the model value.
  std::string obs_client_id;
  // std::vector <std::string> obs_timestamp;
  std::vector <cass_uint32_t> obs_year_month_day;
  std::vector <cass_int64_t> obs_time_of_day;
  std::vector <std::string> obs_configuration;
  std::vector <std::string> obs_features;
  std::vector <std::string> obs_metrics;
  std::vector <std::string> obs_estimates;

  std::vector<std::string> metric_fields_vec;
  std::stringstream str_observation(j);

  //std::string current_date;
  //str_observation >> current_date;
  cass_uint32_t current_date;
  str_observation >> current_date;
  obs_year_month_day.emplace_back(current_date);
  cass_int64_t current_time;
  str_observation >> current_time;
  obs_year_month_day.emplace_back(current_time);

  //std::string current_time;

  //str_observation >> current_time;
  // obs_timestamp.emplace_back(current_date);
  // obs_timestamp.emplace_back(current_time);
  // agora::debug(log_prefix, "Date parsed: ", obs_timestamp[0]);
  // agora::debug(log_prefix, "Time parsed: ", obs_timestamp[1]);

  str_observation >> obs_client_id;
  agora::debug(log_prefix, "Client_id parsed: ", obs_client_id);

  int num_knobs = description.knobs.size();

  while ( num_knobs > 0 )
  {
    std::string current_knob;
    str_observation >> current_knob;
    obs_configuration.emplace_back(current_knob);
    agora::debug(log_prefix, "Knob parsed: ", current_knob);
    num_knobs--;
  }

  int num_features = description.features.size();

  while ( num_features > 0 )
  {
    std::string current_feature;
    str_observation >> current_feature;
    obs_features.emplace_back(current_feature);
    agora::debug(log_prefix, "Feature parsed: ", current_feature);
    num_features--;
  }

  int num_metrics = metric_to_be_analyzed.size();

  while ( num_metrics > 0 )
  {
    std::string current_metric;
    str_observation >> current_metric;
    obs_metrics.emplace_back(current_metric);
    agora::debug(log_prefix, "Metrics parsed: ", current_metric);
    num_metrics--;
  }

  num_metrics = metric_to_be_analyzed.size();

  // variable to understand if the current row from trace was from a training phase
  // It would have ALL the estimates to "N/A"
  // In that case the current row in analysis can be discarded because there is no way
  // of computing the residuals. In the beholder we validate the model, thus the estimates...
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
    agora::debug(log_prefix, "Estimate parsed: ", current_estimate);
    num_metrics--;
  }

  // Check if we have consistency in the quantity of measures parsed,
  // i.e. if the vector of metric names is as big as the one of the observed metrics
  // and is the same size as the number of metrics enabled for the beholder for the current application
  if ((obs_metrics.size() != obs_estimates.size()) || (obs_metrics.size() != metric_to_be_analyzed.size()))
  {
    agora::warning(log_prefix, "Error in the parsed observation, mismatch in the number of fields.");
    return;
  }

  // if the current row from trace is from a training phase discard this row and go to the next;
  if (is_training_row)
  {
    agora::debug(log_prefix, "Discarding current row because it was from a training phase");
    return;
  }

  agora::debug(log_prefix, "Done with the parsing of the current row from trace for client : ", obs_client_id, ", starting the validation and insertion of the metrics into the residual buffers.");


  // Insert the residuals in the right residual maps structure
  // In this phase I have not the information about the names of the metrics from the trace
  // But I parsed them all (the beholder-enabled ones), and I know they are inserted in the trace in alphabetical order
  // according to "description.metrics". So basically I know which metric is the first one
  // (and which estimate is the first one), which is the second one...
  // I kept the same map structure of the residuals computation for the 1st step of CDT.
  // As of now I chose to maintain the mapping to have the metric name built in the structure.
  auto name_ref = metric_to_be_analyzed.begin();

  for (auto index = 0; index < metric_to_be_analyzed.size(); index++, std::advance(name_ref, 1))
  {
    // Check whether the metric in analysis was available and valid or if it was a disabled one-->"null" in trace-->parsed as "N/A"
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
        agora::warning(log_prefix, "Error in the parsed observation, mismatch between the observed (!N/A) and predicted (N/A) metric.");
        return;
      }
    }
    // to catch the case in which the metric was null but the estimate was present (not null). Theoretically impossible by design.
    else if (obs_metrics[index] == "N/A")
    {
      agora::warning(log_prefix, "Error in the parsed observation, mismatch between the observed (N/A) and predicted (!N/A) metric.");
      return;
    }

    // if we arrive here then the parsed metric should be valid (one of the enabled ones at least)

    // NB: note that the residual is computed with abs()!!
    auto current_residual = abs(std::stof(obs_estimates[index]) - std::stof(obs_metrics[index]));
    agora::debug(log_prefix, "Current residual for metric ", *name_ref, " is: ", current_residual);

    auto search = client_residuals_map.find(*name_ref);
    // Load the ici_cdt_map for the current metric
    // This is needed to get the timestamp of the window where the change was detected.
    // Here we need to save the element before that window in a vector (1st in pair),
    // and the elements after that window in another corresponding vector (2nd in pair)
    auto search_ici_map = ici_cdt_map.find(*name_ref);

    if (search != client_residuals_map.end() && search_ici_map != ici_cdt_map.end())
    {
      agora::debug(log_prefix, "metric ", *name_ref, " already present, filling buffer");

      // metric already present, need to add to the buffer the new residual
      // compare timestamp: if before the change insert in the 1st vector of the pair
      if (obs_year_month_day[index] < search_ici_map->second.front_year_month_day)
      {
        // if the current date is older than the one from the first element of the
        // change window then add it to the "before" change window vector.
        search->second.first.emplace_back(current_residual);
      }
      else if (obs_year_month_day[index] == search_ici_map->second.front_year_month_day)
      {
        // if the current observation date is the same as the date of the 1st element of the
        // change window then compare the time. If the current observation's timestamp
        // comes first in the day then add it to the "before" change window vector
        if (obs_time_of_day[index] < search_ici_map->second.front_time_of_day)
        {
          search->second.first.emplace_back(current_residual);
        }

        // if after the change insert in the 2nd vector of the pair
      }
      else if (obs_year_month_day[index] > search_ici_map->second.back_year_month_day)
      {
        // if the current date is newer than the one from the last element of the
        // change window then add it to the "after" change window vector.
        search->second.second.emplace_back(current_residual);
      }
      else if (obs_year_month_day[index] == search_ici_map->second.back_year_month_day)
      {
        // if the current observation date is the same as the date of the last element of the
        // change window then compare the time. If the current observation's timestamp
        // comes later in the day then add it to the "after" change window vector
        if (obs_time_of_day[index] > search_ici_map->second.back_time_of_day)
        {
          search->second.second.emplace_back(current_residual);
        }
      }
    }
    // if we did not find any key with the current metric's name under analysis in the struct
    // then theoretically we should initialize the corresponding struct for the metric
    else if (search_ici_map != ici_cdt_map.end())
    {
      // check that this first value inserted is actually before the change window
      // insert the current observation in the vector of the residuals before the change
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

      if (obs_year_month_day[index] < search_ici_map->second.front_year_month_day)
      {
        // if the current date is older than the one from the first element of the
        // change window then add it to the "before" change window vector.
        valid_metric = true;
      }
      else if (obs_year_month_day[index] == search_ici_map->second.front_year_month_day)
      {
        // if the current observation date is the same as the date of the 1st element of the
        // change window then compare the time. If the current observation's timestamp
        // comes first in the day then add it to the "before" change window vector
        if (obs_time_of_day[index] < search_ici_map->second.front_time_of_day)
        {
          valid_metric = true;
        }
      }

      // if the first observation is before the change window then initialize its struct
      if (valid_metric)
      {
        agora::debug(log_prefix, "creation of buffer for metric and first insertion: ", *name_ref);
        // need to create the mapping for the current metric. It's the first time you meet this metric
        std::vector<float> temp_vector_before;
        std::vector<float> temp_vector_after;
        temp_vector_before.emplace_back(current_residual);
        auto temp_pair = std::make_pair(temp_vector_before, temp_vector_after);
        client_residuals_map.emplace(*name_ref, temp_pair);
      }
      else
      {
        agora::debug(log_prefix, "Skipping the creation of buffer for metric ", *name_ref, " because we do not have observations before the hypothetical change window");
      }
    }
    else
    {
      agora::warning(log_prefix, "Error: no \"ici_cdt_map\" struct found for the metric: ", *name_ref);
      return;
    }
  }

  agora::pedantic(log_prefix, ":", obs_client_id, ": successfully parsed and inserted into buffers the observation: ", j);
}
