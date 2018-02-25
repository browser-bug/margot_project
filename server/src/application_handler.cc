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
#include <cassert>
#include <cstdint>
#include <random>

#include "application_handler.hpp"
#include "logger.hpp"




inline std::size_t rand_between( const std::size_t min, const std::size_t max )
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(min, max);
  return dis(gen);
}


using namespace margot;

RemoteApplicationHandler::RemoteApplicationHandler( const std::string& application_name )
  : status(ApplicationStatus::CLUELESS), description(application_name)
{}


void RemoteApplicationHandler::welcome_client( const std::string& client_name, const std::string& application_name )
{
  // this section is critical ( we need to guard it )
  std::lock_guard<std::mutex> lock(mutex);

  // for sure we have to register the new client
  active_clients.emplace(client_name);

  // we need to decide which actions we need to do
  const bool need_to_restore_this_object = status == ApplicationStatus::CLUELESS;
  const bool need_to_store_client = status == ApplicationStatus::LOADING;
  const bool need_to_send_model = status == ApplicationStatus::WITH_MODEL;
  const bool need_to_send_configuration =  status == ApplicationStatus::EXPLORING;

  // ------------------------------------------------------------- CASE 1: we need to restore the applicatin status
  if (need_to_restore_this_object)
  {
    // load the object
    info("Handler ", application_name, ": the recovery process is started");
    status = ApplicationStatus::LOADING;

    // first of all we need to load the application description, otherwise we cannot
    // insteract with storage and we need them to generater further data structure
    description = io::storage.load_description(application_name);
    description.application_name = application_name;
    const bool description_is_usable = !(description.knobs.empty() || description.metrics.empty());

    // if we have a description, we might have a model if we are lucky
    bool model_is_usable = false;
    if (description_is_usable)
    {
      model = io::storage.load_model(description.application_name);
      const std::size_t theoretical_number_of_columns = description.knobs.size()
                                                        + description.features.size()
                                                        + (2 * description.metrics.size());
      model_is_usable = static_cast<std::size_t>(model.column_size()) == theoretical_number_of_columns;
    }

    // if we don't have a model then we might have a doe going on
    bool we_have_configurations_to_explore = false;
    if (description_is_usable && (!model_is_usable))
    {
      doe = io::storage.load_doe(description.application_name);
      we_have_configurations_to_explore = !doe.required_explorations.empty();
    }

    // if we have the model, we should broadcast it to the clients
    if (model_is_usable)
    {
      info("Handler ", description.application_name, ": recovered a model from storage, broadcasting to clients");
      status = ApplicationStatus::WITH_MODEL;
      send_model("margot/" + description.application_name + "/model");
      pending_clients.clear();
      status = ApplicationStatus::WITH_MODEL;
      return;
    }

    // if we have configurations to explore, let's start with them
    if (we_have_configurations_to_explore)
    {
      info("Handler ", description.application_name, ": recovered a doe from storage, dispatching configurations");
      status = ApplicationStatus::EXPLORING;

      for ( const auto& client : pending_clients )
      {
        send_configuration(client);
      }

      send_configuration(client_name);
      pending_clients.clear();
      return;
    }

    // ok, check if we have to ask for information
    if (!description_is_usable)
    {
      info("Handler ", description.application_name, ": this is a shiny new application, ask \"", client_name, "\" information");
      information_client = client_name;      // we want to know which is the client to speak with
      pending_clients.emplace(client_name);  // once we have the information, we need to provide to him configurations
      io::remote.send_message({{"margot/" + description.application_name + "/" + client_name + "/info"}, ""});
      return;
    }

    // this situation is a bit strange, becuase it means that we have an application
    // description, but we don't have neither a model nor configurations to explore.
    // Something went wrong in the previous run, not sure what, so our only solution
    // is to drop all the tables and start again to ask for information.
    warning("Handler ", description.application_name, ": inconsistent storage infromation, drop existing data and ask \"", client_name, "\" information");
    io::storage.erase(description.application_name);
    io::remote.send_message({{"margot/" + description.application_name + "/" + client_name + "/info"}, ""});
    pending_clients.emplace(client_name);
    description.knobs.clear();
    description.features.clear();
    description.metrics.clear();
    information_client = client_name;
    return; // we have done what we can...
  }

  // ------------------------------------------------------------- CASE 2: we are recovering the object
  if (need_to_store_client) // a doe is not available yet
  {
    pending_clients.emplace(client_name);
  }

  // ------------------------------------------------------------- CASE 3: we are exploring some configurations
  if (need_to_send_configuration)
  {
    send_configuration(client_name);
  }

  // ------------------------------------------------------------- CASE 4: we are building the model
  // sooner or later we broadcast the model, no need to do anything

  // ------------------------------------------------------------- CASE 5: we actually have a model
  if (need_to_send_model)
  {
    send_model("margot/" + description.application_name + "/" + client_name + "/model");
  }

}





void RemoteApplicationHandler::process_info( const std::string& info_message )
{
  // those information are useful only to build the doe
  std::string doe_strategy = "rgam";
  int number_observations = 1;

  // check if we are actually thinking about getting the information
  std::lock_guard<std::mutex> lock(mutex);

  if ( (status != ApplicationStatus::LOADING) || (information_client.empty()) )
  {
    return; // we are not interested on this message
  }

  // free the string with the information client
  information_client.clear();

  // if they are useful, we have to process the string
  info("Handler ", description.application_name, ": parsing the information of the application");
  static constexpr char line_delimiter = '@';
  static constexpr int header_size = 10; // characters
  std::stringstream stream(info_message);
  std::string info_element = {};

  while (std::getline(stream, info_element, line_delimiter))
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

  // check if the application description make sense

  // -- CASE 1: the information are not ok
  // there is no a trivial solution to this case, the best that we can
  // do is to keep asking information to random clients until something ok
  // happens. This statements holds even if we have kust one client
  if ( description.knobs.empty() || description.metrics.empty() )
  {
    information_client = *std::next(std::begin(active_clients), rand_between(0, active_clients.size()));
    io::remote.send_message({{"margot/" + description.application_name + "/" + information_client + "/info"}, ""});
    return;
  }


  // ---------------------------------------------------------------------------------------------- This is to play with doe parameters
  if (doe_strategy.compare("full_factorial") == 0)
  {
    doe.create<DoeStrategy::FULL_FACTORIAL>(description, number_observations);
  }
  else
  {
    warning("Handler ", description.application_name, ": unable to create doe strategy \"", doe_strategy, "\", using full-factorial");
    doe.create<DoeStrategy::FULL_FACTORIAL>(description, number_observations);
  }


  // once that we have creatd the doe, we need to create or store infomation
  io::storage.store_description(description);
  io::storage.store_doe(description, doe);
  io::storage.create_trace_table(description);


  // now, we need to send configurations
  info("Handler ", description.application_name, ": starting the Design Space Exploration");
  status = ApplicationStatus::EXPLORING;

  for ( const auto& client : pending_clients )
  {
    send_configuration(client);
  }

  pending_clients.clear();
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
  std::unique_lock<std::mutex> guard(mutex);

  // check if we can store the information in the application trace
  const bool store_observation = status != ApplicationStatus::CLUELESS && status != ApplicationStatus::LOADING;
  if (store_observation)
  {
    io::storage.insert_trace_entry(description, timestamp + ",'" + client_id + "'," + configuration + "," + features + "," + metrics);
  }

  // check if we actually ask client id to explore the given configuration
  const auto configuration_it = assigned_configurations.find(client_id);
  const bool is_assigned_conf = configuration_it != assigned_configurations.end() ?
                                configuration_it->second.compare(configuration) == 0 : false;


  // if assigned, update the doe of the application
  bool we_need_to_build_model = false;
  if (is_assigned_conf)
  {
    const auto doe_it = doe.required_explorations.find(configuration);
    if (doe_it != doe.required_explorations.end())
    {
      // decrement the doe counter
      doe_it->second--;

      // update it also in the storage
      io::storage.update_doe(description, doe_it->first + "," + std::to_string(doe_it->second));

      // check if we need to remove the configuration
      if (doe_it->second == 0)
      {
        info("Handler ", description.application_name, ": terminated the exploration of configuration \"", doe_it->first, "\", ", doe.required_explorations.size(), " explorations to model");
        doe.next_configuration = doe.required_explorations.erase(doe_it);

        if (doe.next_configuration == doe.required_explorations.end())
        {
          doe.next_configuration = doe.required_explorations.begin();
        }
      }

      // check if this is the last configuration to be explored
      if (!doe.required_explorations.empty())
      {
        send_configuration(client_id);
      }
      else
      {
        we_need_to_build_model = true;
        status = ApplicationStatus::BUILDING_MODEL;
      }
    }
  }


  // if we don't need to generate the model, we are done
  if (!we_need_to_build_model)
  {
    return;
  }


  // otherwise we need to generate the model
  info("Handler ", description.application_name, ": generating the required predictions...");
  model_t temp_model;
  temp_model.create(description);

  // since it takes time, better to unlock it
  guard.unlock();
  io::storage.store_model(description, temp_model, "_temporary"); // in this way it doesnt overlap with model
  temp_model.knowledge.clear();

  // actually build the model
  info("Handler ", description.application_name, ": building the model...");
  io::builder(description);

  // then read the model from the storage
  temp_model = io::storage.load_model(description.application_name,  "_temporary");


  // eventually we have to change the status back
  guard.lock();

  // check if we are still building the model
  if (status == ApplicationStatus::BUILDING_MODEL)
  {
    model = std::move(temp_model);
    io::storage.store_model(description, model);
    status = ApplicationStatus::WITH_MODEL;
    send_model("margot/" + description.application_name + "/model");
  }
  else
  {
    model.knowledge.clear();
    description.clear();
  }

}


void RemoteApplicationHandler::bye_client( const std::string& client_name )
{
  // notify the fact
  info("Handler ", description.application_name, ": goodbye client \"", client_name, "\"");

  // this section is critical ( we need to guard it )
  std::lock_guard<std::mutex> guard(mutex);

  // first things, first: remove the client from the active clients
  const auto active_it = active_clients.find(client_name);

  if (active_it != active_clients.end())
  {
    active_clients.erase(active_it);
  }

  // if we are exploring remove the client from the assigned configurations
  if ( status == ApplicationStatus::EXPLORING )
  {
    const auto conf_it = assigned_configurations.find(client_name);

    if (conf_it != assigned_configurations.end())
    {
      assigned_configurations.erase(conf_it);
    }
  }

  // ------------------------------------------------------------- SPECIAL CASE 1: it was the last client
  if (active_clients.empty())
  {
    info("Handler ", description.application_name, ": everybody left, resetting the object");

    // if nobody is dealing with the application information, we can delete them
    if ((status != ApplicationStatus::LOADING) && (status != ApplicationStatus::BUILDING_MODEL))
    {
      description.clear();
      model.knowledge.clear();
      doe.required_explorations.clear();
      doe.next_configuration = doe.required_explorations.end();
    }

    // after dealing with the real objects, clear the support structure
    active_clients.clear();
    pending_clients.clear();
    assigned_configurations.clear();
    information_client.clear();

    // change the status back to clueless
    status = ApplicationStatus::CLUELESS;
  }

  // ------------------------------------------------------------- SPECIAL CASE 2: it was the information client
  if (information_client.compare(client_name) == 0)
  {
    // we should hire someone else to do it ( the first on the active list )
    information_client = *active_clients.begin();
    io::remote.send_message({{"margot/" + description.application_name + "/" + information_client + "/info"}, ""});
    info("Handler ", description.application_name, ": \"", information_client, "\" is the new information client");
  }
}
