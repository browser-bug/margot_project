/* beholder/application_handler_beholder.hpp
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


#ifndef MARGOT_BEHOLDER_APPLICATION_HANDLER_HDR
#define MARGOT_BEHOLDER_APPLICATION_HANDLER_HDR

#include <string>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include <cassert>

#include "agora/virtual_io.hpp"
#include "agora/common_objects.hpp"

namespace beholder
{

  using application_description_t = agora::application_description_t;

  // for the beholder we are just interested in knowing whether the application has the model or not
  // the beholder will start only when the application applies the model received from agorà
  enum class ApplicationStatus : uint_fast8_t
  {
    READY,
    WITH_MODEL,
    COMPUTING
  };


  class RemoteApplicationHandler
  {


    private:

      // to protect the data structure
      std::mutex mutex;

      // to handle the progress of the elaboration
      ApplicationStatus status;

      // these are the data structures that actually have information
      // about the application behavior
      application_description_t description;

      std::vector<std::string> metric_names;


      // these are the function used to communicate using MQTT topics

      // send a command to all the clients running the application
      inline void send_margot_command( const std::string& command )
      {
        // send the command
        agora::io::remote.send_message({{"margot/" + description.application_name + "/commands"}, command});
      }

      // send an application-specific command to agora
      inline void send_agora_command( const std::string& command )
      {
        // send the command
        agora::io::remote.send_message({{"agora/" + description.application_name + "/commands"}, command});
      }

    public:

      RemoteApplicationHandler( const std::string& application_name );

      void new_observation( const std::string& values );

  };

}

#endif // MARGOT_BEHOLDER_APPLICATION_HANDLER_HDR
