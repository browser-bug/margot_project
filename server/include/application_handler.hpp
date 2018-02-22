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
#include <atomic>
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

      std::atomic_flag spinlock;
      const std::string application_name;
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
      model_t model;
      doe_t doe;



      inline void lock( void )
      {
        while (spinlock.test_and_set(std::memory_order_acquire));
      }

      inline void unlock( void )
      {
        spinlock.clear(std::memory_order_release);
      }

      inline configuration_t get_next( void )
      {
        doe.next_configuration++;
        if (doe.next_configuration == doe.doe.end())
        {
          doe.next_configuration = doe.doe.begin();
        }
        auto configuration_to_send = doe.next_configuration->first;
        return configuration_to_send;
      }

      inline void send_configuration( const std::string& client_name )
      {
        if (!doe.doe.empty())
        {

          // get the configuration to explore
          auto&& next_configuration = get_next();

          // replace the coma with spaces
          std::replace(next_configuration.begin(), next_configuration.end(), ',', ' ' );

          // update the assigned configurations
          assigned_configurations[next_configuration] = client_name;

          // send the configuration
          io::remote.send_message({{"margot/" + application_name + "/" + client_name + "/explore"},next_configuration});
        }
      }

      inline void send_model( const std::string& topic_name ) const
      {
        io::remote.send_message({topic_name,model.join_model()});
      }

      void build_model( void );

    public:


      RemoteApplicationHandler( const std::string& application_name );


      void welcome_client( const std::string& client_name );


      void bye_client( const std::string& client_name );


      void process_info( const std::string& info );



  };

}

#endif // MARGOT_AGORA_APPLICATION_HANDLER_HDR
