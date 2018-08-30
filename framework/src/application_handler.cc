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
#include <sstream>
#include <algorithm>
#include <limits>
#include <cmath>

#include "agora/application_handler.hpp"
#include "agora/logger.hpp"







using namespace agora;

RemoteApplicationHandler::RemoteApplicationHandler( const std::string& application_name )
  : status(ApplicationStatus::CLUELESS), description(application_name), model_iteration_number(1)
{}


void RemoteApplicationHandler::welcome_client( const std::string& client_name, const std::string& application_name )
{
  // this section is critical ( we need to guard it )
  std::unique_lock<std::mutex> guard(mutex);

  // for sure we have to register the new client
  active_clients.emplace(client_name);

  // we need to decide which actions we need to do
  const bool need_to_restore_this_object = status == ApplicationStatus::CLUELESS;
  const bool need_to_ask_information = status == ApplicationStatus::ASKING_FOR_INFORMATION && information_client.empty();
  const bool need_to_send_model = status == ApplicationStatus::WITH_MODEL;
  const bool need_to_send_configuration =  status == ApplicationStatus::EXPLORING;

  // ------------------------------------------------------------- CASE 1: we need to restore the applicatin status
  if (need_to_restore_this_object)
  {
    // load the object
    info("Handler ", application_name, ": detected a new application, attempt to recover status from storage...");
    status = ApplicationStatus::RECOVERING;

    // since interaction with storage could be long, we need to release the lock
    guard.unlock();

    // first of all we need to load the application description, otherwise we cannot
    // insteract with storage and we need them to generate further data structures
    description = io::storage.load_description(application_name);
    description.application_name = application_name;
    const bool description_is_usable = !(description.knobs.empty() || description.metrics.empty());

    // if we have a description, we might have a model if we are lucky
    bool model_is_usable = false;

    if (description_is_usable)
    {
      model = io::storage.load_model(description);
      model_is_usable = !model.knowledge.empty();
    }

    // if we don't have a model then we might have a doe going on
    bool we_have_configurations_to_explore = false;

    if (description_is_usable && (!model_is_usable))
    {
      doe = io::storage.load_doe(description.application_name);
      we_have_configurations_to_explore = !doe.required_explorations.empty();
    }

    // reaquire the lock to change the data structure
    guard.lock();
    info("Handler ", description.application_name, ": recovery process terminated");

    // if all the application exited, we might quit right here
    if (active_clients.empty())
    {
      info("Handler ", description.application_name, ": nobody is alive animore, clearing this handler");
      clear();
      return;
    }

    // if we have the model, we should broadcast it to the clients
    if (model_is_usable)
    {
      info("Handler ", description.application_name, ": known application, broadcasting model");
      status = ApplicationStatus::WITH_MODEL;
      send_model("margot/" + description.application_name + "/model");
      return;
    }

    // if we have configurations to explore, let's start with them
    if (we_have_configurations_to_explore )
    {
      info("Handler ", description.application_name, ": known application, resuming the DSE");
      status = ApplicationStatus::EXPLORING;

      for ( const auto& client : active_clients )
      {
        send_configuration(client);
      }

      return;
    }

    // at this point we need to ask for information
    // because we don't have any clue
    if (!description_is_usable)
    {
      info("Handler ", description.application_name, ": this is a shiny new application");
      status = ApplicationStatus::ASKING_FOR_INFORMATION;
      ask_information();
      return;
    }

    // if reach this point, the this situation is a bit strange, becuase it means that we have an application
    // description, but we don't have neither a model nor configurations to explore.
    // Something went wrong in the previous run, not sure what, so our only solution
    // is to drop all the tables and start again to ask for information.
    warning("Handler ", description.application_name, ": inconsistent storage infromation, drop existing data");
    io::storage.erase(description.application_name);
    status = ApplicationStatus::ASKING_FOR_INFORMATION;
    ask_information();


    // and we might destroy the configuration that we have
    auto&& application_name = description.application_name;
    description.clear();
    description.application_name = application_name;
    return; // we have done what we can...
  }

  // ------------------------------------------------------------- CASE 2: we are recovering the object
  if (need_to_ask_information) // a doe is not available yet and we don't have any information
  {
    ask_information();
  }

  // ------------------------------------------------------------- CASE 3: we are exploring some configurations
  if (need_to_send_configuration)
  {
    send_configuration(client_name);
  }

  // ------------------------------------------------------------- CASE 5: we actually have a model
  if (need_to_send_model)
  {
    send_model("margot/" + description.application_name + "/" + client_name + "/model");
  }

}





void RemoteApplicationHandler::process_info( const std::string& info_message )
{
  // check if we are actually thinking about getting the information
  std::unique_lock<std::mutex> guard(mutex);

  if ( status != ApplicationStatus::ASKING_FOR_INFORMATION || (information_client.empty()) )
  {
    return; // we are not interested on this message
  }

  // free the string with the information client
  // and change the status in building doe
  information_client.clear();

  // if they are useful, we have to process the string
  info("Handler ", description.application_name, ": parsing the information of the application");
  static constexpr char line_delimiter = '@';
  static constexpr int header_size = 10; // characters (it could be best handled, but yeah...)
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
            description.doe_name = info_element.substr(header_size);
          }
          else
          {
            if (line_topic.compare("n_point_d ") == 0)
            {
              description.number_point_per_dimension = info_element.substr(header_size);
            }
            else
            {
              if (line_topic.compare("n_obs_p   ") == 0)
              {
                description.number_observations_per_point = info_element.substr(header_size);
              }
              else
              {
                if (line_topic.compare("min_dist  ") == 0)
                {
                  description.minimum_distance = info_element.substr(header_size);
                }
              }
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
  // happens. This statements holds even if we have just one client
  if ( description.knobs.empty() || description.metrics.empty() || description.number_point_per_dimension.empty() || description.number_observations_per_point.empty() || description.doe_name.empty() || description.minimum_distance.empty() )
  {
    warning("Handler ", description.application_name, ": we received inconsistent information, ask again");

    if (!active_clients.empty())
    {
      ask_information();
    }

    return;
  }

  // -- CASE 2: the information are ok
  // we need to compute and store everything
  // this could last long, so we release the lock
  status = ApplicationStatus::BUILDING_DOE;
  guard.unlock();

  // We let the plugins that model the metrics to generate the configurations
  // required to drive the model, therefore we need to:

  info("Handler ", description.application_name, ": storing the application description");
  io::storage.store_description(description);

  // create the table required to store the execution trace of the application
  info("Handler ", description.application_name, ": creating the required containers in the storage");
  io::storage.create_trace_table(description);
  io::storage.store_doe(description, {});  // we want just to create the doe table TODO: rename this method

  // create and store the requested predictions
  model.create(description);
  io::storage.store_model(description, model);
  model.clear();

  // now we call the plugin that generates the DoE
  info("Handler ", description.application_name, ": generating the DoE");
  io::doe_generator(description, 0);

  // now we need to retrieve the required configuration to explore
  doe = io::storage.load_doe(description.application_name);

  // we are now ready to change the state and see what happens
  guard.lock();

  // if all the application exited, we might quit right here
  if (active_clients.empty())
  {
    info("Handler ", description.application_name, ": nobody is alive animore, clearing this handler");
    clear();
    return;
  }

  // in this case we have configuration to send to the clients
  if (!doe.required_explorations.empty())
  {
    info("Handler ", description.application_name, ": starting the Design Space Exploration");
    status = ApplicationStatus::EXPLORING;

    for ( const auto& client : active_clients )
    {
      send_configuration(client);
    }
  }
  else
  {
    // no one has generated any configuration for me and i don't have a model
    // i don't know what to do, i just panic and start to cry...
    warning("Handler ", description.application_name, ": nobody has configurations for me to explore and neither a model to use... i give up and start crying :(");
  }
}


void RemoteApplicationHandler::new_observation( const std::string& values )
{
  // declare the fields of the incoming message
  std::string client_id;
  std::string timestamp;
  std::string configuration;
  std::string features;
  std::string metrics;

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

  // append the coma to connect the different the features with the metrics
  if (!features.empty())
  {
    features.append(",");
  }

  // this is a critical section
  std::unique_lock<std::mutex> guard(mutex);

  // check if we can store the information in the application trace
  if (status == ApplicationStatus::EXPLORING || status == ApplicationStatus::WITH_MODEL)
  {
    io::storage.insert_trace_entry(description, timestamp + ",'" + client_id + "'," + configuration + "," + features + metrics);
  }
  else
  {
    return; // we are not able to do anything with those information
  }

  // state the boolean variable to state if the client has been assigned to us
  // we start assuming that they are the same
  bool is_assigned_conf = true;

  // search for the client in our records
  const auto configuration_it = assigned_configurations.find(client_id);

  // check if we assigned to him a configuration
  if ( configuration_it != assigned_configurations.end())
  {
    // get the configurations
    std::string observed_configuration = configuration;
    std::string assigned_configuration = configuration_it->second;

    // get the configuration in a parsable format
    std::replace(observed_configuration.begin(), observed_configuration.end(), ',', ' ' );
    std::replace(assigned_configuration.begin(), assigned_configuration.end(), ',', ' ' );

    // parse the configuration values
    std::stringstream oc(observed_configuration);
    std::stringstream ac(assigned_configuration);

    // loop over the sowtware knobs
    for ( const auto& knob : description.knobs )
    {
      // parse the knob as double
      double obs_val, ass_val;
      oc >> obs_val;
      ac >> ass_val;

      // check if they are the same
      if (std::abs(obs_val - ass_val) > std::numeric_limits<double>::epsilon())
      {
        is_assigned_conf = false; // due to different configuration
        break;
      }
    }
  }
  else
  {
    is_assigned_conf = false; // due to unknown client
  }

  // this variable states if we need to rebuild the model
  bool we_need_to_build_model = false;

  // if assigned, update the doe of the application
  if (is_assigned_conf)
  {
    const auto doe_it = doe.required_explorations.find(configuration_it->second);

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
      }
    }
  }


  // if we don't need to generate the model, we are done
  if (!we_need_to_build_model)
  {
    return;
  }

  // otherwise, we need to generate the model
  // since it could take a while, we need to
  // release the lock
  status = ApplicationStatus::BUILDING_MODEL;
  guard.unlock();

  // actually build the model
  info("Handler ", description.application_name, ": learning the application knowledge... (it may take a while)");
  io::model_generator(description, model_iteration_number);
  ++model_iteration_number;

  // then read the model from the storage
  model = io::storage.load_model(description);

  // now we need to retrieve the required configuration to explore (if any)
  doe = io::storage.load_doe(description.application_name);

  // check if we actually have configuration to explore or the model
  const bool with_configuration_to_explore = !doe.required_explorations.empty();
  const bool with_model = !model.knowledge.empty();

  // now we are ready to take some decision
  guard.lock();

  // if all the application exited, we might quit right here
  if (active_clients.empty())
  {
    info("Handler ", description.application_name, ": nobody is alive animore, clearing this handler");
    clear();
    return;
  }

  // this variable test whether we need to generate another DoE to explore
  bool generate_additional_doe = false;

  // otherwise, check if we have configuration to explore
  if (with_configuration_to_explore)
  {
    info("Handler ", description.application_name, ": no application knowledge available, a plugin requested additional observations");
    status = ApplicationStatus::EXPLORING;

    for ( const auto& client : active_clients )
    {
      send_configuration(client);
    }
  }
  else
  {
    // if we have a model it is fine
    if (with_model)
    {
      info("Handler ", description.application_name, ": we have the application knowledge");
      status = ApplicationStatus::WITH_MODEL;
      send_model("margot/" + description.application_name + "/model");
    }
    else
    {
      // no one has generated any configuration for me and i don't have a model
      // we restart the dse
      warning("Handler ", description.application_name, ": no application knowledge available, restarting the DSE");
      generate_additional_doe = true;
    }
  }

  // if we have a model or a doe, we are done
  if (!generate_additional_doe)
  {
    return;
  }

  // otherwise, we need to restart the doe
  // this could last long, so we release the lock
  status = ApplicationStatus::BUILDING_DOE;
  guard.unlock();

  info("Handler ", description.application_name, ": re-generating the DoE");
  io::doe_generator(description, 0);

  // now we need to retrieve the required configuration to explore
  doe = io::storage.load_doe(description.application_name);

  // we are now ready to change the state and see what happens
  guard.lock();

  // if all the application exited, we might quit right here
  if (active_clients.empty())
  {
    info("Handler ", description.application_name, ": nobody is alive animore, clearing this handler");
    clear();
    return;
  }

  // in this case we have configuration to send to the clients
  if (!doe.required_explorations.empty())
  {
    info("Handler ", description.application_name, ": re-starting the Design Space Exploration");
    status = ApplicationStatus::EXPLORING;

    for ( const auto& client : active_clients )
    {
      send_configuration(client);
    }
  }
  else
  {
    // no one has generated any configuration for me and i don't have a model
    // i don't know what to do, i just panic and start to cry...
    warning("Handler ", description.application_name, ": nobody has configurations for me to explore and neither a model to use... i give up and start crying :(");
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

  // we are allowed to clear the object only if there is nothing pending
  if (active_clients.empty() && status != ApplicationStatus::CLUELESS && status != ApplicationStatus::RECOVERING && status != ApplicationStatus::BUILDING_DOE && status != ApplicationStatus::BUILDING_MODEL )
  {
    info("Handler ", description.application_name, ": this was the last client, no pending operation, clearing this handler");
    clear();
    return;
  }

  // ------------------------------------------------------------- SPECIAL CASE 2: it was the information client
  if (information_client.compare(client_name) == 0)
  {
    if (!active_clients.empty())
    {
      ask_information();
    }
  }
}
