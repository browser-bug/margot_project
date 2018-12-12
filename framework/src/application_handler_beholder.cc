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

#include "beholder/application_handler_beholder.hpp"
#include "beholder/parameters_beholder.hpp"
#include "beholder/observation_data.hpp"
#include "beholder/ici_cdt.hpp"
#include "beholder/hypothesis_test.hpp"
#include "agora/logger.hpp"


using namespace beholder;

RemoteApplicationHandler::RemoteApplicationHandler( const std::string& application_name )
  : status(ApplicationStatus::READY), description(application_name)
{
  agora::info("New beholder application handler created for application: ", application_name);

  // load the application description, even though we are just interested in the metrics
  description = agora::io::storage.load_description(application_name);
  agora::debug("Number of total metrics: ", description.metrics.size());
  agora::debug("Window size: ", Parameters_beholder::window_size);
  agora::debug("Number of windows used for training: ", Parameters_beholder::training_windows);


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
  parse_observation(observation, values);

  // fill in the buffers with the current observations
  fill_buffers(observation);

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
      i.second.clear();
      // go to the next metric (next iteration of the for cycle), if available, until the cycle has finished
      continue;
    }

    agora::debug("i.second.size(): ", i.second.size());

    // if any buffer is filled in then perform the ici cdt on it
    if (i.second.size() == Parameters_beholder::window_size)
    {
      agora::pedantic("Buffer for metric ", i.first, " filled in, starting CDT on the current window.");

      // Check if we already have a ici_cdt_map for the current metric, if not create it
      auto search_ici_map = ici_cdt_map.find(i.first);

      // If it finds the ici_cdt_map for the current metric then use it
      if (search_ici_map != ici_cdt_map.end())
      {
        agora::pedantic("Continuing CDT for metric ", i.first);
        change_detected = IciCdt::perform_ici_cdt(search_ici_map->second, i.second);
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
        agora::pedantic("Initialized structure for metric ", i.first, ", starting CDT! ");
        change_detected = IciCdt::perform_ici_cdt(new_ici_struct, i.second);
        ici_cdt_map.emplace(i.first, new_ici_struct);
        // It cannot detect a change if this is the first full window for that metric, we are in training of cdt
        // so basically avoid the if to check for the boolean "change_detected"
      }

      // if the change has been detected save the name of the (first) metric for which the change has been detected
      // This could be useful later on.
      // Save also the timestamp of the first and last element of the window
      // to be able to pinpoint where the change was detected. This will be needed
      // in the 2nd step of the CDT.
      if (change_detected)
      {
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
      agora::info("Something is wrong, the list of clients received from the DB is empty!");
      return;
    }

    agora::debug("Client list without duplicates:");

    for (auto i : clients_list)
    {
      agora::debug(i);
    }

    // initialize timeout shared (consumed) among all the clients
    int timeout = Parameters_beholder::timeout;

    // this counter is to take into account, or better, not to take into account
    // the number of clients which actually gave a useful answer to the hypothesis test
    // We do not take into account the clients for which there were no enough observations
    // to perform the 2nd level test in the count for good/bad clients.
    //int actual_number_of_valid_clients = clients_list.size(); // TODO: do we really need this?

    // cycle over all the clients of the specific application
    for (auto i : clients_list)
    {
      // NB: here I could choose to avoid iterating over some possibly already blacklisted
      // clients. Of course in this case I should check whether the element iterator already
      // belongs in the blacklist like this:
      // if (clients_blacklist.count(i) == 1) {
      //     continue;  // this sould continue the iterator "i"
      // }
      // But pay attention, then I need to change the way I compute the bad clients percentage
      // below, in particular I need to add the already blacklisted clients, then it becomes:
      // float bad_clients_percentage = (((bad_clients_list.size() + clients_blacklist.size()) / clients_list.size())*100);

      // initialized as a duplicate of reference_metric_names, that is to say all the metrics enabled for the
      // beholder analysis this list contains the metrics yet to be tested with the hypothesis test, so once a
      // metric has been analyzed it will be removed from this structure
      std::set<std::string> metric_to_be_analyzed;
      for (auto j : reference_metric_names){
          metric_to_be_analyzed.emplace(j);
      }

      bool confirmed_change = false;
      bool valid_client = true;

      // String containing the 'select' part of the query to Cassandra
      // to only receive the metrics enabled for the beholder analysis by the user
      std::string query_select;

      while (metric_to_be_analyzed.size() != 0 && !confirmed_change){
          // data structure to save the residuals for the specific application-client pair before the change
          // the structure is organized as a map which maps the name of the metric to a pair
          // The first element of the pair is a vector containing the residuals for that metric before the change windows
          // The second element of the pair is a vector containing the residuals for that metric after the change windows
          std::unordered_map<std::string, std::pair < std::vector<float>, std::vector<float>>> client_residuals_map;

          // preparing the 'select' query for Cassandra for the 2nd step of cassandra
          // It is done here on the basis of whixch metrics are left to be analyzed
          query_select = "day, time, client_id, ";

          for (auto i : description.knobs)
          {
            query_select.append(i.name);
            query_select.append(",");
          }

          for (auto i : description.features)
          {
            query_select.append(i.name);
            query_select.append(",");
          }

          for (auto i : metric_to_be_analyzed)
          {
            query_select.append(i);
            query_select.append(",");
          }

          for (auto i : metric_to_be_analyzed)
          {
            query_select.append("model_");
            query_select.append(i);
            query_select.append(",");
          }

          // to remove the last comma
          query_select.pop_back();

          //observations_list = agora::io::storage.load_client_observations(description.application_name, "alberto_Surface_Pro_2_6205", query_select);
          observations_list = agora::io::storage.load_client_observations(description.application_name, i, query_select);
          agora::debug("\nParsing the trace for client ", i);

          // cycle over each row j of the trace for each client i (for the current application)
          for (auto j : observations_list)
          {
            parse_and_insert_observations_for_client_from_trace(client_residuals_map, j, metric_to_be_analyzed);
          }

          // let's analyze if any of the metrics has enough observations to perform the test
          // remove the metrics which cannot be analyzed from the client_residuals_map
          for (auto it = client_residuals_map.begin(); it != client_residuals_map.end();){
              if (it->second.first.size() < Parameters_beholder::min_observations || it->second.second.size() < Parameters_beholder::min_observations){
                  it = client_residuals_map.erase(it);
              }
              else {
                  ++it;
              }
          }

          // if there is at least a metric on which we can perform the hypothesis test
          if (client_residuals_map.size() > 0){
              // Execute on the specific client the 2nd step of CDT: hypothesis TEST
              confirmed_change = HypTest::perform_hypothesis_test(client_residuals_map);
          }

          if (confirmed_change){
              // as soon as the change is confirmed by any of the metric then exit to classify the current
              // client under analysis as a bad one and move to the next client (if any left)
              break;
          } else {
              // remove the metrics present in the client_residuals_map from the metric_to_be_analyzed
              for (auto it : client_residuals_map){

              }
              // if metric_to_be_analyzed still not empty then if the timeout is not over then wait
              // if the timeout was over then break but before breaking out set valid_client to false
          }
      }

      if (valid_client){
          // according to the quality (GOOD=no_change/BAD=confirmed_change) of the currently analyzed client, enqueue it in the good/bad_clients_list
          if (confirmed_change)
          {
            bad_clients_list.emplace(i);
          }
          else
          {
            good_clients_list.emplace(i);
          }
      } else {
          timeout_clients_list.emplace(i);
      }



    }

    // compute the percentage of bad clients and compare it wrt the predefined threshold
    float bad_clients_percentage = (((bad_clients_list.size() + timeout_clients_list.size())/ clients_list.size()) * 100);

    // Once I know what the outcome of the hypothesis TEST is, re-acquire the lock
    guard.lock();

    if (timeout_clients_list.size() == 0){
        // TODO: warn with a message
    }


    // if the number % of bad clients is over the user-set threshold then trigger the re-training
    // This also covers the case in which all the clients behave badly (100%)
    if (bad_clients_percentage > Parameters_beholder::bad_clients_threshold)
    {
      // Basically in this case the change detected by the 1st level cdt was confirmed by the hypothesys test

      // need to trigger RE-training
      // this automatically deals with the deletion of the model and of the trace and with the reset of the doe
      // in this version the trace is deleted just from the top element in the table up to the last element of the training window
      if (Parameters_beholder::no_trace_drop)
      {
        auto search_ici_map = ici_cdt_map.find(change_metric);

        if (search_ici_map != ici_cdt_map.end())
        {
          send_agora_command("retraining " + std::to_string(search_ici_map->second.back_year_month_day) + "," + std::to_string(search_ici_map->second.back_time_of_day));
        }
      }
      else
      {
        // delete the whole trace
        send_agora_command("retraining");
      }

      // TODO: destroy this application handler

    }
    else
    {
      // this is the case in which either there are some bad clients but they are below threshold,
      // or there are no bad clients at all. The behavior is the same.
      // The change is discarded (but so are also the values coming from these bad_clients, if any).

      // This is also the path taken if there is no answer to the 2nd level step because the timeout
      // has run out.

      // if there are some bad clients then put them in the blacklist
      if (bad_clients_list.size() > 0)
      {
        for (auto i : bad_clients_list)
        {
          clients_blacklist.emplace(i);
        }
      }

      // reset the 1st level cdt data structure for all the metrics which have a structure
      // just keep the test configuration settings (training) and reset the rest.

      // for every CDT data structure for this application (meaning for every metric that was analyzed in the ICI CDT)
      for (auto i : ici_cdt_map)
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

      // set the status back to ready
      status = ApplicationStatus::READY;
    }
  }

  // The following command will (probably) be never used.
  // if (false /*need to enable metrics*/)
  // {
  //   send_margot_command("metrics_on");
  // }
}


// method to parse the received observation from mqtt messages
void RemoteApplicationHandler::parse_observation(Observation_data& observation, const std::string& values)
{
  // declare the temporary variables to store the fields of the incoming message
  std::string metrics;
  std::string metric_fields;
  std::string estimates;
  std::string user_enabled_metrics;

  // parse the timestamp and the client_id
  std::stringstream stream(values);
  stream >> observation.timestamp;
  agora::debug("Timestamp: ", observation.timestamp);
  stream >> observation.client_id;
  agora::debug("client_id: ", observation.client_id);

  // check whether the client which sent the current observation is in the blacklist.
  // if that's the case then discard the observation, otherwise keep parameter_string
  auto search = clients_blacklist.find(observation.client_id);

  if (search != clients_blacklist.end())    // if client name found in the blacklist than return
  {
    agora::info("Observation from client ", observation.client_id, " rejected because blacklisted client");
    return;
  }

  // get the observed values
  stream >> metrics;
  agora::debug("metrics: ", metrics);

  // gets the model values
  stream >> estimates;
  agora::debug("estimates: ", estimates );

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
  agora::debug("metric_fields: ", metric_fields);

  // if this is the first message then parse also the last part of the message where the heel enqueues the whole list of user-enabled-beholder-metrics
  // if this list is not present than there is the assumption that all the metrics should be analized, even though the user did not explicitely stated that
  if (reference_metric_names.size() == 0)
  {
    stream >> user_enabled_metrics;

    //if there is no list than get the reference metrics from the application description, it is assumed that all of them are enabled
    if (user_enabled_metrics == "")
    {
      for (auto i : description.metrics)
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

  for (auto i : observation.metric_fields_vec)
  {
    agora::debug("metric_fields separated: ", i);
  }

  // build the vector of observed metrics provided in the observation
  std::stringstream ssm(metrics);

  while ( ssm.good() )
  {
    std::string substr;
    getline( ssm, substr, ',' );
    observation.metrics_vec.push_back( std::stof(substr) );
  }

  for (auto i : observation.metrics_vec)
  {
    agora::debug("metrics separated: ", i);
  }

  // build the vector of observed metrics provided in the observation
  std::stringstream ssme(estimates);

  while ( ssme.good() )
  {
    std::string substr;
    getline( ssme, substr, ',' );
    observation.estimates_vec.push_back( std::stof(substr) );
  }

  for (auto i : observation.estimates_vec)
  {
    agora::debug("estimates separated: ", i);
  }


  // Check if we have consistency in the quantity of measures received,
  // i.e. if the vector of metric names is as big as the one of the observed metrics and the one of the estimates
  if ((observation.metric_fields_vec.size() != observation.metrics_vec.size()) || (observation.metrics_vec.size() != observation.estimates_vec.size()))
  {
    agora::info("Error in the observation received, mismatch in the number of fields.");
    return;
  }
}

// method which adds the parsed values to the current observation to the respective buffers
void RemoteApplicationHandler::fill_buffers(const Observation_data& observation)
{
  // Insert the residuals in the right buffers according to the metric name
  for (auto index = 0; index < observation.metric_fields_vec.size(); index++)
  {
    // NB: note that the residual is computed with abs()!!
    // TODO: is this correct?
    auto current_residual = abs(observation.estimates_vec[index] - observation.metrics_vec[index]);
    agora::debug("Current residual for metric ", observation.metric_fields_vec[index], " is: ", current_residual);

    auto search = residuals_map.find(observation.metric_fields_vec[index]);
    auto search_counter = residuals_map_counter.find(observation.metric_fields_vec[index]);

    if ((search != residuals_map.end()) && (search_counter != residuals_map_counter.end()))
    {
      agora::debug("metric ", observation.metric_fields_vec[index], " already present, filling buffer");
      // metric already present, need to add to the buffer the new residual
      auto temp_pair = std::make_pair(current_residual, observation.timestamp);
      search->second.emplace_back(temp_pair);
      // increase the counter of observations for the current metric
      search_counter->second = search_counter->second + 1;
    }
    else if (((search == residuals_map.end()) && (search_counter != residuals_map_counter.end())) ||
             ((search != residuals_map.end()) && (search_counter == residuals_map_counter.end())))
    {
      // If the current metric in analysis in only found in one of the two maps then there is an error
      // because the maps should contain the same keys, since their insertion is basically parallel.
      agora::info("Error: mismatch between the two residual map structures.");
      return;
    }
    else
    {
      agora::debug("creation of buffer for metric and first insertion: ", observation.metric_fields_vec[index]);
      // need to create the mapping for the current metric. It's the first time you meet this metric
      auto temp_pair = std::make_pair(current_residual, observation.timestamp);
      std::vector<std::pair <float, std::string>> temp_vector;
      temp_vector.emplace_back(temp_pair);
      residuals_map.emplace(observation.metric_fields_vec[index], temp_vector);
      // start counting the observations for the current metrics
      residuals_map_counter.emplace(observation.metric_fields_vec[index], 1);
    }
  }
}

// method which receives as parameters the list of observations from a specific client from the trace
// and parses them and inserts its residuals in the corresponding map structure
void RemoteApplicationHandler::parse_and_insert_observations_for_client_from_trace(std::unordered_map<std::string, std::pair < std::vector<float>, std::vector<float>>>& client_residuals_map,
    const observation_t j, const std::set<std::string>& metric_to_be_analyzed)
{
  agora::debug("\nString from trace to be parsed: ", j);
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
  // agora::debug("Date parsed: ", obs_timestamp[0]);
  // agora::debug("Time parsed: ", obs_timestamp[1]);

  str_observation >> obs_client_id;
  agora::debug("Client_id parsed: ", obs_client_id);

  int num_knobs = description.knobs.size();

  while ( num_knobs > 0 )
  {
    std::string current_knob;
    str_observation >> current_knob;
    obs_configuration.emplace_back(current_knob);
    agora::debug("Knob parsed: ", current_knob);
    num_knobs--;
  }

  int num_features = description.features.size();

  while ( num_features > 0 )
  {
    std::string current_feature;
    str_observation >> current_feature;
    obs_features.emplace_back(current_feature);
    agora::debug("Feature parsed: ", current_feature);
    num_features--;
  }

  int num_metrics = metric_to_be_analyzed.size();

  while ( num_metrics > 0 )
  {
    std::string current_metric;
    str_observation >> current_metric;
    obs_metrics.emplace_back(current_metric);
    agora::debug("Metrics parsed: ", current_metric);
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
    agora::debug("Estimate parsed: ", current_estimate);
    num_metrics--;
  }

  // Check if we have consistency in the quantity of measures parsed,
  // i.e. if the vector of metric names is as big as the one of the observed metrics
  // and is the same size as the number of metrics enabled for the beholder for the current application
  if ((obs_metrics.size() != obs_estimates.size()) || (obs_metrics.size() != metric_to_be_analyzed.size()))
  {
    agora::info("Error in the parsed observation, mismatch in the number of fields.");
    return;
  }

  // if the current row from trace is from a training phase discard this row and go to the next;
  if (is_training_row)
  {
    agora::debug("Discarding current row because it was from a training phase");
    return;
  }

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
        agora::info("Error in the parsed observation, mismatch between the observed and predicted metric.");
        return;
      }
    }
    // to catch the case in which the metric was null but the estimate was present (not null). Theoretically impossible by design.
    else if (obs_metrics[index] == "N/A")
    {
      agora::info("Error in the parsed observation, mismatch between the observed and predicted metric.");
      return;
    }

    // if we arrive here then the parsed metric should be valid (one of the enabled ones at least)

    // NB: note that the residual is computed with abs()!!
    auto current_residual = abs(std::stof(obs_estimates[index]) - std::stof(obs_metrics[index]));
    agora::debug("Current residual for metric ", *name_ref, " is: ", current_residual);

    auto search = client_residuals_map.find(*name_ref);
    // Load the ici_cdt_map for the current metric
    // This is needed to get the timestamp of the window where the change was detected.
    // Here we need to save the element before that window in a vector (1st in pair),
    // and the elements after that window in another corresponding vector (2nd in pair)
    auto search_ici_map = ici_cdt_map.find(*name_ref);

    if (search != client_residuals_map.end() && search_ici_map != ici_cdt_map.end())
    {
      agora::debug("metric ", *name_ref, " already present, filling buffer");

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
        agora::debug("creation of buffer for metric and first insertion: ", *name_ref);
        // need to create the mapping for the current metric. It's the first time you meet this metric
        std::vector<float> temp_vector_before;
        std::vector<float> temp_vector_after;
        temp_vector_before.emplace_back(current_residual);
        auto temp_pair = std::make_pair(temp_vector_before, temp_vector_after);
        client_residuals_map.emplace(*name_ref, temp_pair);
      }
      else
      {
        agora::debug("Skipping the creation of buffer for metric ", *name_ref, " because we do not have observations before the hypothetical change window");
      }
    }
    else
    {
      agora::info("Error: no \"ici_cdt_map\" struct found for the metric: ", *name_ref);
      return;
    }
  }
}
