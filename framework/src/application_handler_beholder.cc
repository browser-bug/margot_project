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

#include "beholder/application_handler_beholder.hpp"
#include "beholder/parameters_beholder.hpp"
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
  guard.unlock();

  // data structure to store the residuals from the observations received,
  // i.e. the difference between the predicted value by the model and the actual value.
  // This structure maps every metric (name) to its buffer of residuals.
  // The buffer will be as big as the beholder parameter "window_size" instructs.
  std::unordered_map<std::string, std::vector<float>> residuals_map;

  //TODO: rearrange this parsing after having chosen what the beholder really receives from mqtt.
  // declare the fields of the incoming message
  std::string client_id;
  std::string timestamp;
  //std::string configuration;
  //std::string features;
  std::string metrics;
  std::string metric_fields;
  std::string estimates;

  // parse the message
  std::stringstream stream(values);
  stream >> timestamp;
  agora::debug("Timestamp: ", timestamp);
  stream >> client_id;
  agora::debug("client_id: ", client_id);

  // check whether the client which sent the current observation is in the blacklist.
  // if that's the case then discard the observation, otherwise keep parameter_string
  auto search = clients_blacklist.find(client_id);

  if (search != clients_blacklist.end())    // if client name fouund in the blacklist than return
  {
    agora::info("Observation from client ", client_id, " rejected because blacklisted client");
    return;
  }

  //stream >> configuration;

  // if (!description.features.empty()) // parse the features only if we have them
  // {
  //   stream >> features;
  // }

  // gets the observed values
  stream >> metrics;
  agora::debug("metrics: ", metrics);

  // gets the model values
  stream >> estimates;
  agora::debug("estimates: ", estimates );

  // gets the name of the fields of the metric to be filled in
  // NB: note that the beholder observation message always contains the metric names, also when all the metrics are enabled.
  stream >> metric_fields;
  agora::debug("metric_fields: ", metric_fields);

  // append the coma to connect the different the features with the metrics
  // if (!features.empty())
  // {
  //   features.append(",");
  // }

  // build the vector of metric names provided in the observation
  std::vector<std::string> metric_fields_vec;
  std::stringstream ssmf(metric_fields);

  while ( ssmf.good() )
  {
    std::string substr;
    getline( ssmf, substr, ',' );
    metric_fields_vec.push_back( substr );
  }

  for (auto i : metric_fields_vec)
  {
    agora::debug("metric_fields separated: ", i);
  }

  // build the vector of observed metrics provided in the observation
  std::vector<float> metrics_vec;
  std::stringstream ssm(metrics);

  while ( ssm.good() )
  {
    std::string substr;
    getline( ssm, substr, ',' );
    metrics_vec.push_back( std::stof(substr) );
  }

  for (auto i : metrics_vec)
  {
    agora::debug("metrics separated: ", i);
  }

  // build the vector of observed metrics provided in the observation
  std::vector<float> estimates_vec;
  std::stringstream ssme(estimates);

  while ( ssme.good() )
  {
    std::string substr;
    getline( ssme, substr, ',' );
    estimates_vec.push_back( std::stof(substr) );
  }

  for (auto i : estimates_vec)
  {
    agora::debug("estimates separated: ", i);
  }


  // Check if we have consistency in the quantity of measures received,
  // i.e. if the vector of metric names is as big as the one of the observed metrics and the one of the estimates
  if ((metric_fields_vec.size() != metrics_vec.size()) || (metrics_vec.size() != estimates_vec.size()))
  {
    agora::info("Error in the observation received, mismatch in the number of fields.");
    return;
  }

  // this is a critical section
  guard.lock();

  // Insert the residuals in the right buffers according to the metric name
  for (auto index = 0; index < metric_fields_vec.size(); index++)
  {
    // NB: note that the residual is computed with abs()!!
    // TODO: is this correct?
    auto current_residual = abs(estimates_vec[index] - metrics_vec[index]);
    agora::debug("Current residual for metric ", metric_fields_vec[index], " is: ", current_residual);

    auto search = residuals_map.find(metric_fields_vec[index]);

    if (search != residuals_map.end())
    {
      agora::debug("metric ", metric_fields_vec[index], " already present, filling buffer");
      // metric already present, need to add to the buffer the new residual
      search->second.emplace_back(current_residual);
    }
    else
    {
      agora::debug("creation of buffer for metric and first insertion: ", metric_fields_vec[index]);
      // need to create the mapping for the current metric. It's the first time you meet this metric
      std::vector<float> temp_vector;
      temp_vector.emplace_back(current_residual);
      residuals_map.emplace(metric_fields_vec[index], temp_vector);
    }
  }

  // Check whether one (or more) buffers is (are) filled in
  // up to the beholder's window_size parameter.
  for (auto& i : residuals_map)
  {
    agora::debug("i.second.size(): ", i.second.size());

    if (i.second.size() == Parameters_beholder::window_size)
    {
      agora::pedantic("Buffer for metric ", i.first, " filled in, starting CDT on the current window.");

      // TODO: start computation for CDT

      // TODO at the end of the CDT, empty the filled-in buffer.
      i.second.clear();

      // if (/*CDT positivo*/){
      //     status = ApplicationStatus::COMPUTING;
      // }
    }
  }

  // STEP 2 of CDT: analysis with granularity on the single client

  // UNLOCK the guard because we are accessing the db????

  // if need to start the step 2 of CDT:
  if (true /* status == ApplicationStatus::COMPUTING */)
  {

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
    clients_list = agora::io::storage.load_clients(description.application_name);

    for (auto i : clients_list)
    {
      agora::debug("Client list without duplicates: ", i);
    }

    // cycle over all the clients of the specific application
    for (auto i : clients_list)
    {
      // DB QUERY TEST: query to retrieve all the observations for a pair application-client_name
      // for (auto i: clients_list){
      //     observations_list = agora::io::storage.load_client_observations(description.application_name, i);
      //
      //     // Second level hypothesis test on client-specific observations_list
      //     // Here you basically choose whether each client is bad or not.
      //     // create a counter system for good, bad clients, so that you can choose, at the end of this cycle
      //     // if the number of bad clients is above the predefined threshold and act accordingly
      //     // either blacklisting or trigger re-training or nothing
      // }

      // TODO: this is a test with just one row. Later on wrap this (the following) in a for loop to scan every row.
      // so instead of observations_list[0] there should be observations_list[i]
      std::unordered_map<std::string, std::vector<float>> client_residuals_map;
      //observations_list = agora::io::storage.load_client_observations(description.application_name, "alberto_Surface_Pro_2_6205");
      observations_list = agora::io::storage.load_client_observations(description.application_name, i);
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
        // Shoould I re-gain the lock of the guard to "end" the decisions about this application? The following stuff...

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
