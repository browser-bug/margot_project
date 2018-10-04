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

  // gets the name of the fields of the metric to be filled in (if any, if empty fills in all the metrics of the table normally)
  stream >> metric_fields;
  agora::debug("metric_fields: ", metric_fields);

  // gets the observed values
  stream >> metrics;
  agora::debug("metrics: ", metrics);

  // gets the model values
  stream >> estimates;
  agora::debug("estimates: ", estimates );

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

  // Insert the residuals in the right buffers according to the metric namespace
  for (auto index = 0; index < metric_fields_vec.size(); index++)
  {
    auto current_residual = estimates_vec[index] - metrics_vec[index];
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

      // TODO start computation for CDT

      // TODO at the end of the CDT, empty the filled-in buffer.
      i.second.clear();
    }
  }

  // DB QUERY TEST:
  clients_list = agora::io::storage.load_clients(description.application_name);



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
