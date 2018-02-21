/* agora/remote_handler.hpp
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


#ifndef MARGOT_AGORA_COMMUNICATOR_GENERIC_HDR
#define MARGOT_AGORA_COMMUNICATOR_GENERIC_HDR


#include "safe_queue.hpp"
#include "common_objects.hpp"


namespace margot
{

  class RemoteHandler
  {
    public:

      // this class should not copied or moved around
      RemoteHandler( void ): inbox() {}
      RemoteHandler( const RemoteHandler& ) = delete;
      RemoteHandler( RemoteHandler&& ) = delete;

      using MessageQueue = Queue<message_t>;

      inline bool recv_message( message_t& input_message )
      {
        return inbox.dequeue(input_message);
      }

      virtual void send_message( message_t& output_message ) = 0;

      virtual void subscribe( const std::string& topic) = 0;

      virtual void unsubscribe( const std::string& topic ) = 0;

      virtual void disconnect( void ) = 0;

      virtual ~RemoteHandler( void ) {}

    protected:

      MessageQueue inbox;

  };

}

#endif // MARGOT_AGORA_COMMUNICATOR_GENERIC_HDR
