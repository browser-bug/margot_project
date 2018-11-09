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

#include <string>

#include "agora/safe_queue.hpp"
#include "agora/common_objects.hpp"
#include "agora/logger.hpp"


namespace agora
{

  inline void whitelist( message_t& incoming_string )
  {
    if (incoming_string.topic.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_/") != std::string::npos)
    {
      warning("Input sanitizer: found a non valid character in the topic of the message, i will discard it");
      incoming_string.topic = "margot/error";
      incoming_string.payload = "";
    }

    if (incoming_string.payload.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_ -.,@<>=;()^*+") != std::string::npos)
    {
      warning("Input sanitizer: found a non valid character in the payload message, i will discard it");
      incoming_string.topic = "margot/error";
      incoming_string.payload = "";
    }
  }

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
        const bool rc = inbox.dequeue(input_message);
        whitelist(input_message);
        return rc;
      }

      virtual void send_message( const message_t&& output_message ) = 0;

      virtual void subscribe( const std::string& topic) = 0;

      virtual void unsubscribe( const std::string& topic ) = 0;

      virtual void disconnect( void ) = 0;

      virtual std::string get_my_client_id( void ) const = 0;

      virtual ~RemoteHandler( void ) {}

    protected:

      MessageQueue inbox;

  };

}

#endif // MARGOT_AGORA_COMMUNICATOR_GENERIC_HDR
