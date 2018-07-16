/* beholder/application_handler_beholder.cc
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

#include <ctime>
#include <cassert>
#include <cstdint>
#include <sstream>
#include <algorithm>
#include <limits>

#include "beholder/application_handler_beholder.hpp"
#include "agora/logger.hpp"


using namespace beholder;

RemoteApplicationHandler::RemoteApplicationHandler( const std::string& application_name )
  : status(ApplicationStatus::CLUELESS), description(application_name)
{
  agora::info("New beholder application handler created for application: ", application_name);
}

void RemoteApplicationHandler::new_observation( const std::string& values )
{
  // declare the fields of the incoming message
  std::string client_id;
  std::string timestamp;
  std::string configuration;
  std::string features;
  std::string metrics;
  std::string metric_fields;

  // parse the message
  std::stringstream stream(values);
  stream >> timestamp;
  stream >> client_id;
  stream >> configuration;

  if (!description.features.empty()) // parse the features only if we have them
  {
    stream >> features;
  }

  stream >> metrics;
  stream >> metric_fields; // gets the name of the fields of the metric to be filled in (if any, if empty fills in all the metrics of the table normally)

  // append the coma to connect the different the features with the metrics
  if (!features.empty())
  {
    features.append(",");
  }

  // this is a critical section
  std::unique_lock<std::mutex> guard(mutex);

  if (false /*need to enable metrics*/)
  {
    send_command("metrics_on");
  }

}
