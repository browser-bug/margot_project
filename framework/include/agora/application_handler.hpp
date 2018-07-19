/* agora/application_handler.hpp
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


#ifndef MARGOT_AGORA_APPLICATION_HANDLER_HDR
#define MARGOT_AGORA_APPLICATION_HANDLER_HDR

#include <string>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include <cassert>

#include "agora/virtual_io.hpp"
#include "agora/common_objects.hpp"
#include "agora/doe.hpp"

namespace agora
{

  enum class ApplicationStatus : uint_fast8_t
  {
    CLUELESS,
    RECOVERING,
    ASKING_FOR_INFORMATION,
    BUILDING_DOE,
    EXPLORING,
    BUILDING_MODEL,
    WITH_MODEL,
  };


  class RemoteApplicationHandler
  {


    private:

      // to protect the data structure
      std::mutex mutex;

      // to handle the progress of the elaboration
      ApplicationStatus status;

      // this table contains all the clients of this application
      application_list_t active_clients;

      // this relates a client with the assigned configuration
      application_map_t assigned_configurations;

      // this is the name of the client who is in charge of collecting info
      std::string information_client;


      // these are the data structures that actually have information
      // about the application behavior
      application_description_t description;
      model_t model;
      doe_t doe;

      inline configuration_t get_next( void )
      {
        doe.next_configuration++;

        if (doe.next_configuration == doe.required_explorations.end())
        {
          doe.next_configuration = doe.required_explorations.begin();
        }

        auto configuration_to_send = doe.next_configuration->first;
        return configuration_to_send;
      }

      // these are the function used to communicate using MQTT topics

      // send a configuration to the client
      inline void send_configuration( const std::string& client_name )
      {
        if (!doe.required_explorations.empty())
        {

          // get the configuration to explore
          auto next_configuration = get_next();

          // update the assigned configurations
          assigned_configurations[client_name] = next_configuration;

          // replace the coma with spaces
          std::replace(next_configuration.begin(), next_configuration.end(), ',', ' ' );

          // send the configuration
          io::remote.send_message({{"margot/" + description.application_name + "/" + client_name + "/explore"}, next_configuration});
        }
      }

      // send the model to a specific topic
      inline void send_model( const std::string& topic_name ) const
      {
        io::remote.send_message({topic_name, model.join(description)});
      }

      // ask a client to retrieve information about the application
      inline void ask_information( void )
      {
        assert( !active_clients.empty() );
        information_client = *active_clients.begin();
        io::remote.send_message({{"margot/" + description.application_name + "/" + information_client + "/info"}, ""});
        info("Handler ", description.application_name, ": asking \"", information_client, "\" information");
      }

    public:

      RemoteApplicationHandler( const std::string& application_name );

      void welcome_client( const std::string& client_name, const std::string& application_name );

      void bye_client( const std::string& client_name );

      void process_info( const std::string& info );

      void new_observation( const std::string& values );

      void retraining( void );

      inline ApplicationStatus get_status()
      {
        std::unique_lock<std::mutex> guard(mutex);
        return status;
      }

      inline bool active_clients_empty()
      {
        std::unique_lock<std::mutex> guard(mutex);
        return active_clients.empty();
      }

      inline std::string get_application_name()
      {
        std::unique_lock<std::mutex> guard(mutex);
        return description.application_name;
      }

  };

}

#endif // MARGOT_AGORA_APPLICATION_HANDLER_HDR
