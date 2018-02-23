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

#include "virtual_io.hpp"
#include "common_objects.hpp"
#include "doe.hpp"

namespace margot
{

  enum class ApplicationStatus : uint_fast8_t
  {
    CLUELESS,
    LOADING,
    GENERATING_DOE,
    EXPLORING,
    BUILDING_MODEL, // do not put the client in pending list
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

      // this contains all the applications that are waiting to receive
      // a configuration from the server
      application_list_t pending_clients;

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
          io::remote.send_message({{"margot/" + description.application_name + "/" + client_name + "/explore"},next_configuration});
        }
      }

      inline void send_model( const std::string& topic_name ) const
      {
        io::remote.send_message({topic_name,model.join(description)});
      }

    public:


      RemoteApplicationHandler( const std::string& application_name );


      void welcome_client( const std::string& client_name );


      void bye_client( const std::string& client_name );


      void process_info( const std::string& info );

      void new_observation( const std::string& values );



  };

}

#endif // MARGOT_AGORA_APPLICATION_HANDLER_HDR
