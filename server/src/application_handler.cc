/* agora/application_handler.cc
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

#include "application_handler.hpp"
#include "logger.hpp"


using namespace margot;

RemoteApplicationHandler::RemoteApplicationHandler( const std::string& application_name )
  : spinlock(ATOMIC_FLAG_INIT), status(ApplicationStatus::CLUELESS), description(application_name)
{}

void RemoteApplicationHandler::build_model( void )
{
  //TODO
  warning("Application Client: we don't support the model generation yet");
  //info("Handler ", description.application_name, ": creating and storing the required predictions in the model");
  //model.create(description);
}


void RemoteApplicationHandler::welcome_client( const std::string& client_name )
{
  // for sure we have to register the new client
  lock();
  active_clients.emplace(client_name);
  unlock();

  // ------------------------------------------------------------- CASE 1: this is the first client
  lock();
  const bool clueless = status == ApplicationStatus::CLUELESS;
  if (clueless)
  {
    status = ApplicationStatus::LOADING;
  }
  unlock();
  if (clueless)
  {
    // first of all we need to load the application description, otherwise we cannot do nothing
    description = io::storage.load_description(description.application_name);
    if ( description.knobs.empty() || description.metrics.empty() )
    {
      info("Handler ", description.application_name, ": asking \"", client_name, "\" to provide information");
      lock();
      information_client = client_name;      // we want to know which is the client to speak with
      pending_clients.emplace(client_name);  // once we have the information, we need to provide to him configurations
      io::remote.send_message({{"margot/" + description.application_name + "/" + client_name + "/info"}, ""});
      unlock();
      return; // we cannot do anything
    }


    // if we are lucky we already have the model of the application
    model = io::storage.load_model(description.application_name);
    const int theoretical_number_of_columns = static_cast<int>(description.knobs.size() + description.features.size() + (2*description.metrics.size()));
    if ( model.column_size() == theoretical_number_of_columns)
    {
      info("Handler ", description.application_name, ": recovered model from storage");
      lock();
      status = ApplicationStatus::WITH_MODEL; // change the status to the final one
      pending_clients.clear(); // because we are about to broadcast the model
      unlock();
      send_model("/margot/" + description.application_name + "/model");
      return; // we have done our work
    }

    // if we are a bit less lucky we have at least the doe (with model in db)
    doe = io::storage.load_doe(description.application_name);
    if (!doe.required_explorations.empty()) // if the doe is terminated, we must also have the model
    {
      // we configurations yet to explore, we shall start
      info("Handler ", description.application_name, ": recovered doe from storage");
      lock();
      status = ApplicationStatus::EXPLORING;
      for( const auto& client : pending_clients )
      {
        send_configuration(client);
      }
      send_configuration(client_name);
      pending_clients.clear();
      unlock();
      return; // we have done our work
    }

    return; // we have fone our work
  }

  // ------------------------------------------------------------- CASE 2: we are initializing the object
  lock();
  if ((status == ApplicationStatus::LOADING) || (status == ApplicationStatus::GENERATING_DOE))
  {
    // we can't do anything, put the client in the pending list
    pending_clients.emplace(client_name);
  }
  unlock();

  // ------------------------------------------------------------- CASE 3: we are exploring some configurations
  lock();
  if (status == ApplicationStatus::EXPLORING)
  {
    send_configuration(client_name);
  }
  unlock();

  // ------------------------------------------------------------- CASE 4: we are building the model
  // sooner or later we broadcast the model, no need to do anything

  // ------------------------------------------------------------- CASE 5: we actually have a model
  lock();
  const bool with_model = status == ApplicationStatus::WITH_MODEL;
  unlock();
  if (with_model)
  {
    send_model("margot/" + description.application_name + "/" + client_name + "/model");
  }

}


void RemoteApplicationHandler::bye_client( const std::string& client_name )
{
  // for sure we have to remove the client from the active ones and from the configuration list
  lock();
  const auto active_it = active_clients.find(client_name);
  if (active_it != active_clients.end())
  {
    active_clients.erase(active_it);
  }
  const auto conf_it = assigned_configurations.find(client_name);
  if (conf_it != assigned_configurations.end())
  {
    assigned_configurations.erase(conf_it);
  }
  unlock();

  // ------------------------------------------------------------- CASE 1: it was the last client
  lock();
  if (active_clients.empty())
  {
    info("Handler ", description.application_name, ": everybody left, resetting the object");

    // everybody left, we have to reset the object, unless someone is generating the doe
    if ((status != ApplicationStatus::GENERATING_DOE) && (status != ApplicationStatus::BUILDING_MODEL))
    {
      status = ApplicationStatus::CLUELESS;
      model.knowledge.clear();
      doe.required_explorations.clear();
      assigned_configurations.clear();
      information_client.clear(); // if this happens when we are in the LOADING state
    }
    else
    {
      status = ApplicationStatus::CLUELESS; // we can't free memory, but we can change status
    }
  }
  unlock();

  // ------------------------------------------------------------- CASE 2: it was the information client
  lock();
  const bool is_information_client = information_client.compare(client_name) == 0;
  if (is_information_client)
  {
    // we should hire someone else to do it ( the first on the active list )
    information_client = *active_clients.begin();
    io::remote.send_message({{"margot/" + description.application_name + "/" + information_client + "/info"}, ""});

    info("Handler ", description.application_name, ": goodbye client \"", client_name, "\", requesting info at client \"", information_client, "\"");
  }
  else
  {
    // ------------------------------------------------------------- CASE 3: it is a random client
    // we don't actually care
    info("Handler ", description.application_name, ": goodbye client \"", client_name, "\"");
  }
  unlock();
}


void RemoteApplicationHandler::process_info( const std::string& info_message )
{
  // those information are useful only to build the doe
  std::string doe_strategy = "full_factorial";
  int number_observations = 1;

  // make sure that we want those information
  lock();
  const bool useful = status == ApplicationStatus::LOADING;
  if (useful)
  {
    status = ApplicationStatus::GENERATING_DOE;
  }
  unlock();

  // if we are not looking for them, return
  if (!useful)
  {
    return;
  }

  // otherwise we have to process the string
  info("Handler ", description.application_name, ": parsing the information of the application");
  static constexpr char line_delimiter = '@';
  static constexpr int header_size = 10; // characters
  std::stringstream stream(info_message);
  std::string info_element = {};
  while(std::getline(stream,info_element,line_delimiter))
  {
    // make sure to skip void elements
    if (info_element.empty())
    {
      continue;
    }

    // otherwise get the first character to understand the line
    const std::string line_topic = info_element.substr(0, header_size);
    if (line_topic.compare("knob      ") == 0)
    {
      knob_t new_knob = {};
      new_knob.set(info_element.substr(header_size));
      description.knobs.emplace_back(new_knob);
    }
    else
    {
      if (line_topic.compare("feature   ") == 0)
      {
        feature_t new_feature = {};
        new_feature.set(info_element.substr(header_size));
        description.features.emplace_back(new_feature);
      }
      else
      {
        if (line_topic.compare("metric    ") == 0)
        {
          metric_t new_metric = {};
          new_metric.set(info_element.substr(header_size));
          description.metrics.emplace_back(new_metric);
        }
        else
        {
          if (line_topic.compare("doe       ") == 0)
          {
            doe_strategy = info_element.substr(header_size);
          }
          else
          {
            if (line_topic.compare("num_obser ") == 0)
            {
              std::istringstream ( info_element.substr(header_size) ) >> number_observations;
            }
          }
        }
      }
    }
  }

  // free the name of the client that should inform us about the application
  lock();
  information_client.clear();
  unlock();

  // first of all we have to store the application description
  io::storage.store_description(description);

  // then we need to create the trace table
  io::storage.create_trace_table(description);

  // we have to generate the doe
  info("Handler ", description.application_name, ": generating and storing the doe");
  if (doe_strategy.compare("full_factorial") == 0)
  {
    doe.create<DoeStrategy::FULL_FACTORIAL>(description, number_observations);
  }
  else
  {
    warning("Handler ", description.application_name, ": unable to create doe strategy \"", doe_strategy, "\", using full-factorial");
    doe.create<DoeStrategy::FULL_FACTORIAL>(description, number_observations);
  }

  // we have to store the remaining information
  io::storage.store_doe(description, doe);

  // dispatch to the clients the required information (if there is someone available)
  lock();
  if (status == ApplicationStatus::GENERATING_DOE)
  {
    info("Handler ", description.application_name, ": starting the Design Space Exploration");
    status = ApplicationStatus::EXPLORING;
    for( const auto& client : pending_clients )
    {
      send_configuration(client);
    }
    pending_clients.clear();
  }
  else
  {
    info("Handler ", description.application_name, ": nobody is left, but we have stored the information");
    doe.required_explorations.clear();
    description.clear();
    pending_clients.clear();
  }
  unlock();

}


void RemoteApplicationHandler::new_observation( const std::string& values )
{
  // extract from the values the client_id
  std::string client_id;
  std::string timestamp;
  std::string configuration;
  std::string features;
  std::string metrics;
  std::stringstream stream(values);
  stream >> timestamp;
  stream >> client_id;
  stream >> configuration;
  stream >> features;
  stream >> metrics;

  // this is a critical section
  lock();

  // check if the client_id is actually exploring something
  const auto it = assigned_configurations.find(client_id);

  // check if this is the assigned configuration
  const bool is_assigned_conf = it != assigned_configurations.end() ? it->second.compare(configuration) == 0 : false;

  // check if we are able to accept a new configuration
  const bool we_are_ready = (status != ApplicationStatus::CLUELESS) && (status != ApplicationStatus::LOADING) && (status != ApplicationStatus::GENERATING_DOE);

  // check if we are the one that should build the model
  bool we_have_to_build_the_model = false;

  // check if we need to send another configuration
  if (is_assigned_conf)
  {
    // update the doe
    const auto doe_it = doe.required_explorations.find(configuration);
    if (doe_it != doe.required_explorations.end())
    {
      // update the counter
      doe_it->second--;

      // send the query to update also the fs
      io::storage.update_doe(description, doe_it->first + "," + std::to_string(doe_it->second));

      // check if we need to remove the configuration
      if (doe_it->second == 0)
      {
        info("Handler ", description.application_name, ": terminated the exploration of configuration \"",doe_it->first,"\", ",doe.required_explorations.size()," explorations to model");
        doe.next_configuration = doe.required_explorations.erase(doe_it);
        if (doe.next_configuration == doe.required_explorations.end())
        {
          doe.next_configuration = doe.required_explorations.begin();
        }
      }

      // check if we need to build the model
      we_have_to_build_the_model = doe.required_explorations.empty();
      if (!we_have_to_build_the_model)
      {
        send_configuration(client_id);

      }
      else
      {
        status = ApplicationStatus::BUILDING_MODEL;
      }
    }
  }
  unlock();

  // if we are ready, we can store the information
  if (we_are_ready)
  {
    io::storage.insert_trace_entry(description, timestamp + ",'" + client_id + "'," + configuration + "," + features + "," + metrics);
  }

  // if we have to build the model, we should start
  if (we_have_to_build_the_model)
  {
    info("Handler ", description.application_name, ": building the model...");

    // we actually build the model
    build_model();

    // change the status, we are done (if there is someone alive)
    lock();
    if (status == ApplicationStatus::BUILDING_MODEL)
    {
      status = ApplicationStatus::WITH_MODEL;
    }
    else
    {
      model.knowledge.clear();
      description.clear();
    }
    unlock();
  }

}
