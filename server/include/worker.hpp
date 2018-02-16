/* agora/worker.hpp
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


#ifndef MARGOT_AGORA_WORKER_HDR
#define MARGOT_AGORA_WORKER_HDR

#include <string>
#include <iostream>


#include "virtual_channel.hpp"
#include "remote_handler.hpp"


namespace margot
{

  class Worker
  {
    private:

      VirtualChannel channel;


    public:

      Worker(VirtualChannel channel): channel(channel) {}


      inline void operator()(const message_t& new_message)
      {
        // trivial implementation of the response system
        std::cout << "[" << new_message.topic << "] -> " << new_message.payload << std::endl;

        // this is to test the send of the message
        message_t response_message = {{"margot/anwser"}, {new_message.payload}};
        channel.send_message(response_message);

        // the very first control is for quitting the whole thing
        if (new_message.topic.compare("margot/system") == 0)
        {
          if (new_message.payload.compare("shutdown") ==  0)
          {
            channel.destroy_channel();
          }
        }
      }


  };


}

#endif // MARGOT_AGORA_WORKER_HDR
