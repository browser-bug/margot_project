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

void RemoteApplicationHandler::send_model( const std::string& topic_name )
{
  io::remote.send_message({topic_name,"begin"});
  for( const auto& configuration : model.model_data )
  {
    io::remote.send_message({topic_name,configuration});
  }
  io::remote.send_message({topic_name,"end"});
}

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
        io::remote.send_message({{"margot/" + application_name + "/" + client + "/explore"},get_next()});
      }
      io::remote.send_message({{"margot/" + application_name + "/" + client_name + "/explore"},get_next()});
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
  if (status == ApplicationStatus::LOADING)
  {
    // we can't do anything, put the client in the pending list
    pending_clients.emplace(client_name);
  }
  unlock();

  // ------------------------------------------------------------- CASE 3: we are exploring some configurations
  lock();
  if (status == ApplicationStatus::EXPLORING)
  {
    io::remote.send_message({{"margot/" + application_name + "/" + client_name + "/explore"},get_next()});
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
    // everybody left, we have to reset the object
    info("Handler ", application_name, ": everybody left, resetting the object");
    status = ApplicationStatus::CLUELESS;
    pending_clients.clear();         // should be useless
    assigned_configurations.clear();
    information_client.clear();
    model.clear();
    doe.clear();
    knobs.clear();
    features.clear();
    metrics.clear();
  }
  unlock();

  // ------------------------------------------------------------- CASE 2: it was the information client
  lock();
  const bool is_information_client = information_client.compare(client_name) == 0;
  if (is_information_client)
  {
    // we have to reset previous information
    knobs.clear();
    features.clear();
    metrics.clear();

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
