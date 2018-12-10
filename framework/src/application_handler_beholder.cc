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
  RemoteApplicationHandler::parse_observation(observation, values);

  // fill in the buffers with the current observations
  RemoteApplicationHandler::fill_buffer(observation);

  // just universal variables to control the change detection and thus the flow of the CDT itself
  // since we do not care to actually check all the metrics, but it is enough a metric (the first)
  // to trigger the 2nd step of the hierarchiacla CDT.
  bool change_detected = false;
  std::string change_metric;

  // variables to save the time range in which the change was detected.
  // This will be useful in the 2nd step of the hierarchical CDT.
  std::string change_window_timestamp_front = "";
  std::string change_window_timestamp_back = "";


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
        change_metric = i.first; // TODO: do we need thisw????
        change_window_timestamp_front = i.second.front().second;
        change_window_timestamp_back = i.second.back().second;
        // set the status variable according so that the lock can be released
        status = ApplicationStatus::COMPUTING;
        // Empty the (filled-in) buffer for the current metric.
        i.second.clear();
      }
    }
  }

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

    // DB QUERY TEST: query to retrieve all the distinct clients which are running a specif application
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

    // cycle over all the clients of the specific application
    for (auto i : clients_list)
    {
      // data structure to save the residuals for the specific application-client pair
      std::unordered_map<std::string, std::vector<float>> client_residuals_map;

      //observations_list = agora::io::storage.load_client_observations(description.application_name, "alberto_Surface_Pro_2_6205", query_select);
      observations_list = agora::io::storage.load_client_observations(description.application_name, i, query_select);
      agora::debug("\nParsing the trace for client ", i);

      // NB: here I could choose to avoid iterating over some possibly already blacklisted
      // clients. Of course in this case I should check whether the element iterator already
      // belongs in the blacklist like this:
      // if (clients_blacklist.count(i) == 1) {
      //     continue;  // this sould continue the iterator "i"
      // }
      // But pay attention, then I need to change the way I compute the bad clients percentage
      // below, in particular I need to add the already blacklisted clients, then it becomes:
      // float bad_clients_percentage = (((bad_clients_list.size() + clients_blacklist.size()) / clients_list.size())*100);

      // cycle over each row j of the trace for each client i (for the current application)
      for (auto j : observations_list)
      {

        agora::debug("\nString from trace to be parsed: ", j);
        // TODO: parse the string. Taking into account the number of enabled metrics in the current observation.
        // we need to know which metric(s) we have to retrieve and compare with the model estimation
        std::string obs_client_id;
        std::vector <std::string> obs_timestamp;
        std::vector <std::string> obs_configuration;
        std::vector <std::string> obs_features;
        std::vector <std::string> obs_metrics;
        std::vector <std::string> obs_estimates;

        std::vector<std::string> metric_fields_vec;
        std::stringstream str_observation(j);

        std::string current_date;
        str_observation >> current_date;
        std::string current_time;
        str_observation >> current_time;
        obs_timestamp.emplace_back(current_date);
        obs_timestamp.emplace_back(current_time);
        agora::debug("Date parsed: ", obs_timestamp[0]);
        agora::debug("Time parsed: ", obs_timestamp[1]);

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

        int num_metrics = description.metrics.size();

        while ( num_metrics > 0 )
        {
          std::string current_metric;
          str_observation >> current_metric;
          obs_metrics.emplace_back(current_metric);
          agora::debug("Metrics parsed: ", current_metric);
          num_metrics--;
        }

        num_metrics = description.metrics.size();

        // variable to understand if the current row from trace was from a training phase
        // It would have all the estimates to "N/A"
        // In that case the current row in analysis can be discarded because there is way
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
        // i.e. if the vector of metric names is as big as the one of the observed metrics and is the same size as the number of metrics for the current application
        if ((obs_metrics.size() != obs_estimates.size()) || (obs_metrics.size() != description.metrics.size()))
        {
          agora::info("Error in the parsed observation, mismatch in the number of fields.");
          return;
        }

        // if the current row from trace is from a training phase discard this row and go to the next;
        if (is_training_row)
        {
          agora::debug("Discarding current row because it was from a training phase");
          continue;
        }

        // Insert the residuals in the right residual maps structure
        // In this phase I have not the information about the names of the metrics from the trace
        // But I parsed them all, and I know they are inserted in the trace in alphabetical order
        // according to "description.metrics". So basically I know which metric is the first one
        // (and which estimate is the first one), which is the second one...
        // I kept the same map structure of the residuals computation for the 1st step of CDT
        // but theoretically here I know that I just have a certain number of metrics and where
        // to position them. I could have build a different structure, a simpler one.
        // As of now I chose to maintain the mapping to have the metric name built in the structure.
        for (auto index = 0; index < description.metrics.size(); index++)
        {
          // Check whether the metric in analysis was available or if it was a disabled one-->"null" in trace-->parsed as "N/A"
          if (obs_estimates[index] == "N/A")
          {
            // for the current version, if a metric is disabled its corresponding prediction must be disable too
            if (obs_metrics[index] == "N/A")
            {
              // skip the comparison on this metric between it was not enabled
              continue;
            }
            else
            {
              agora::info("Error in the parsed observation, mismatch between the observed and predicted metric.");
              return;
            }
          }
          // to catch the case in which the estimate was null but the metric was present. Theoretically impossible by design.
          else if (obs_metrics[index] == "N/A")
          {
            agora::info("Error in the parsed observation, mismatch between the observed and predicted metric.");
            return;
          }

          // if we arrive here then the parsed metric should be valid (one of the enabled ones at least)

          // NB: note that the residual is computed with abs()!!
          // TODO: is this correct?
          auto current_residual = abs(std::stof(obs_estimates[index]) - std::stof(obs_metrics[index]));
          agora::debug("Current residual for metric ", description.metrics[index].name, " is: ", current_residual);

          auto search = client_residuals_map.find(description.metrics[index].name);

          if (search != client_residuals_map.end())
          {
            agora::debug("metric ", description.metrics[index].name, " already present, filling buffer");
            // metric already present, need to add to the buffer the new residual
            search->second.emplace_back(current_residual);
          }
          else
          {
            agora::debug("creation of buffer for metric and first insertion: ", description.metrics[index].name);
            // need to create the mapping for the current metric. It's the first time you meet this metric
            std::vector<float> temp_vector;
            temp_vector.emplace_back(current_residual);
            client_residuals_map.emplace(description.metrics[index].name, temp_vector);
          }
        }

        // TODO: Execute the 2nd step of CDT: hypothesis TEST


        // Once I know what the outcome of the hypothesis TEST is,
        // TODO: Shoould I re-gain the lock of the guard to "end" the decisions about this application?
        // Or can I keep asynchronous w/o lock? Does it even make a difference?

        // guard.lock();

        // according to the quality (GOOD/BAD) of the currently analyzed client, enqueue it in the good/bad_clients_list
        if (false /* client is bad */)
        {
          // bad_clients_list.emplace(/*client_name*/);
        }
        else
        {
          // good_clients_list.emplace(/*client_name*/);
        }

      }

    }



    // compute the percentage of bad clients and compare it wrt the predefined threshold
    float bad_clients_percentage = ((bad_clients_list.size() / clients_list.size()) * 100);

    if (bad_clients_percentage > Parameters_beholder::bad_clients_threshold)
    {
      // TODO: trigger retraining and put status back to READY
      // send_agora_command("retraining");
      // TODO: actions post retraining
      // reset blacklist?
      // reset observation buffers?
      // ApplicationStatus to READY?
      // set change_detected to false?
    }

    // TODO: manage the unlock/lock of this DB access phase
  }



  if (false /*need to enable metrics*/)
  {
    send_margot_command("metrics_on");
  }

  if (false /*need to trigger RE-training*/)
  {
    send_agora_command("retraining");
    // TODO: actions post retraining
    // reset blacklist?
    // reset observation buffers?
    // ApplicationStatus to READY?
  }

}


// method to parse the received observation
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

  if (search != clients_blacklist.end())    // if client name fouund in the blacklist than return
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

    // preparing the 'select' query for Cassandra for the 2nd step of cassandra
    // It is done here to only compute the string once and save resources and time
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

    for (auto i : reference_metric_names)
    {
      query_select.append(i);
      query_select.append(",");
    }

    for (auto i : reference_metric_names)
    {
      query_select.append("model_");
      query_select.append(i);
      query_select.append(",");
    }

    // to remove the last comma
    query_select.pop_back();
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
void RemoteApplicationHandler::fill_buffer(Observation_data& observation)
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
