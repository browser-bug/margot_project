/* agora/paho_remote_implementation.hpp
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


#ifndef MARGOT_AGORA_PAHO_REMOTE_IMPLEMENTATION_HDR
#define MARGOT_AGORA_PAHO_REMOTE_IMPLEMENTATION_HDR


#include <string>
#include <atomic>


extern "C"
{
#include "MQTTClient.h"
}


#include "remote_handler.hpp"


namespace margot
{

  class PahoClient: public RemoteHandler
  {
    private:

      MQTTClient client;
      bool is_connected;
      uint8_t qos_level;
      std::atomic_flag send_spinlock;


    public:

      PahoClient( const std::string& broker_address, const uint8_t qos_level = 1, const std::string& username = "", const std::string& password = "");
      ~PahoClient( void );

      // don't copy or move this object, things explode otherwise
      PahoClient( const PahoClient& ) = delete;
      PahoClient( PahoClient&& ) = delete;


      void send_message( message_t& output_message );

      void subscribe( const std::string& topic );

      void unsubscribe( const std::string& topic );

      void disconnect( void );

  };

}


#endif // MARGOT_AGORA_PAHO_REMOTE_IMPLEMENTATION_HDR
