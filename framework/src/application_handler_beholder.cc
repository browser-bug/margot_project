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

  while( ssmf.good() )
  {
    std::string substr;
    getline( ssmf, substr, ',' );
    metric_fields_vec.push_back( substr );
  }
  for (auto i : metric_fields_vec){
      agora::debug("metric_fields separated: ", i);
  }



  // build the vector of observed metrics provided in the observation
  std::vector<float> metrics_vec;
  std::stringstream ssm(metrics);

  while( ssm.good() )
  {
    std::string substr;
    getline( ssm, substr, ',' );
    metrics_vec.push_back( std::stof(substr) );
  }
  for (auto i : metrics_vec){
      agora::debug("metrics separated: ", i);
  }

  // build the vector of observed metrics provided in the observation
  std::vector<float> estimates_vec;
  std::stringstream ssme(estimates);

  while( ssme.good() )
  {
    std::string substr;
    getline( ssme, substr, ',' );
    estimates_vec.push_back( std::stof(substr) );
  }
  for (auto i : estimates_vec){
      agora::debug("estimates separated: ", i);
  }



  // this is a critical section
  guard.lock();

  // Once the parsing of the new observation has been done, you need to check whether the
  // client is present in the clients_blacklist. If so, return, otherwise keep going.
  // Later you need to insert the new residuals in the right buffer(s).
  // Once you do that, you need to check whether one (or more) buffers is (are) filled in
  // up to the beholder's window_size parameter.
  // If the buffer (vector inside map) in not filled in yet than return,
  // else you start the computation.

  if (false /*need to enable metrics*/)
  {
    send_margot_command("metrics_on");
  }

  if (false /*need to trigger RE-training*/)
  {
    send_agora_command("retraining");
  }

}
