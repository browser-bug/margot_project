/* agora/virtual_channel.hpp
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


#ifndef MARGOT_AGORA_VIRTUAL_CHANNEL_HDR
#define MARGOT_AGORA_VIRTUAL_CHANNEL_HDR

#include <memory>
#include <cassert>

#include "remote_handler.hpp"
#include "paho_remote_implementation.hpp"


namespace margot
{

  class VirtualChannel
  {
    private:

      std::shared_ptr< RemoteHandler > channel;

    public:

      template< class T, class ...Ts >
      inline void create( const Ts& ... remote_arguments )
      {
        channel.reset( new T(remote_arguments...));
      }

      inline void destroy_channel( void )
      {
        assert(channel && "Error: destroy on an empty channel");
        channel->disconnect();
      }

      inline bool recv_message( message_t& output_message )
      {
        assert(channel && "Error: recv on an empty channel");
        return channel->recv_message(output_message);
      }

      inline void send_message( message_t& input_message )
      {
        assert(channel && "Error: send on an empty channel");
        channel->send_message(input_message);
      }

      inline void subscribe( const std::string& topic)
      {
        assert(channel && "Error: subscribe on an empty channel");
        channel->subscribe(topic);
      }

      inline void unsubscribe( const std::string& topic )
      {
        assert(channel && "Error: unsubscribe on an empty channel");
        channel->subscribe(topic);
      }

      inline std::string get_my_client_id( void ) const
      {
        assert(channel && "Error: unable to get the client id from an empty channel");
        return channel->get_my_client_id();
      }

  };
}


#endif // MARGOT_AGORA_VIRTUAL_CHANNEL_HDR
