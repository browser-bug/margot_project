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

#include "application_handler.hpp"
#include "logger.hpp"


using namespace margot;

RemoteApplicationHandler::RemoteApplicationHandler( const std::string& application_name )
  : spinlock(ATOMIC_FLAG_INIT), application_name(application_name), status(ApplicationStatus::CLUELESS)
{}

void RemoteApplicationHandler::build_model( void )
{
  //TODO
  warning("Application Client: we don't support the model generation yet");
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
    // if we are lucky we already have the model of the application
    model = io::storage.load_model(application_name);
    if (model.usable())
    {
      info("Handler ", application_name, ": recovered model from storage");
      lock();
      status = ApplicationStatus::WITH_MODEL; // change the status to the final one
      pending_clients.clear(); // because we are about to broadcast the model
      unlock();
      send_model("/margot/" + application_name + "/model");
      return; // we have done our work
    }
    else
    {
      model.clear(); // we don't need it yet
    }

    // if we are a bit less lucky we have at least the doe (with model in db)
    doe = io::storage.load_doe(application_name);
    if (doe.usable())
    {
      // we configurations yet to explore, we shall start
      info("Handler ", application_name, ": recovered doe from storage");
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

    // we are actually clueless
    info("Handler ", application_name, ": asking \"", client_name, "\" to provide information");
    lock();
    information_client = client_name;      // we want to know which is the client to speak with
    pending_clients.emplace(client_name);  // once we have the information, we need to provide to him configurations
    io::remote.send_message({{"margot/" + application_name + "/" + client_name + "/info"}, ""});
    unlock();
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
    send_model("margot/" + application_name + "/" + client_name + "/model");
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
    info("Handler ", application_name, ": everybody left, resetting the object");

    // everybody left, we have to reset the object, unless someone is generating the doe
    if (status != ApplicationStatus::GENERATING_DOE)
    {
      status = ApplicationStatus::CLUELESS;
      model.clear();
      doe.clear();
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
    io::remote.send_message({{"margot/" + application_name + "/" + information_client + "/info"}, ""});

    info("Handler ", application_name, ": goodbye client \"", client_name, "\", requesting info at client \"", information_client, "\"");
  }
  else
  {
    // ------------------------------------------------------------- CASE 3: it is a random client
    // we don't actually care
    info("Handler ", application_name, ": goodbye client \"", client_name, "\"");
  }
  unlock();
}


void RemoteApplicationHandler::process_info( const std::string& info_message )
{
  // those information are useful only to build the doe
  application_knobs_t knobs;
  application_features_t features;
  application_metrics_t metrics;
  std::string&& doe_strategy = "full_factorial";
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
  info("Handler ", application_name, ": parsing the information of the application");
  static constexpr char line_delimiter = '@';
  static constexpr int header_size = 10; // characters
  std::stringstream stream(info_message);
  std::string&& info_element = {};
  while(std::getline(stream,info_element,line_delimiter))
  {
    const std::string line_topic = info_element.substr(0, header_size);
    if (line_topic.compare("knob      ") == 0)
    {
      knob_t&& new_knob = {};
      new_knob.set(info_element.substr(header_size));
      knobs.emplace_back(new_knob);
    }
    else
    {
      if (line_topic.compare("feature   ") == 0)
      {
        feature_t&& new_feature = {};
        new_feature.set(info_element.substr(header_size));
        features.emplace_back(new_feature);
      }
      else
      {
        if (line_topic.compare("metric    ") == 0)
        {
          metric_t&& new_metric = {};
          new_metric.set(info_element.substr(header_size));
          metrics.emplace_back(new_metric);
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

  // we have to generate and store the model
  info("Handler ", application_name, ": creating and storing the required predictions in the model");
  model.create<DoeStrategy::FULL_FACTORIAL>(knobs, features, metrics);
  io::storage.store_model(application_name, model);
  model.clear(); // right now we don't need it

  // free the useless information from memory
  io::storage.store_features(application_name, features);
  features.clear();
  io::storage.store_metrics(application_name, metrics);
  metrics.clear();

  // we have to generate the doe
  info("Handler ", application_name, ": generating and storing the doe");
  if (doe_strategy.compare("full_factorial") == 0)
  {
    doe.create<DoeStrategy::FULL_FACTORIAL>(knobs, number_observations);
  }
  else
  {
    warning("Handler ", application_name, ": unable to create doe strategy \"", doe_strategy, "\", using full-factorial");
    doe.create<DoeStrategy::FULL_FACTORIAL>(knobs, number_observations);
  }

  // we have to store the remaining information
  io::storage.store_doe(application_name, doe);
  io::storage.store_knobs(application_name, knobs);
  knobs.clear();

  // dispatch to the clients the required information (if there is someone available)
  lock();
  if (status == ApplicationStatus::GENERATING_DOE)
  {
    info("Handler ", application_name, ": starting the Design Space Exploration");
    status = ApplicationStatus::EXPLORING;
    for( const auto& client : pending_clients )
    {
      send_configuration(client);
    }
    pending_clients.clear();
  }
  else
  {
    info("Handler ", application_name, ": nobody is left, but we have stored the information");
    doe.clear();
  }
  unlock();

}
