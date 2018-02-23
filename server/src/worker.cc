/* agora/worker.cc
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

// those two headers are retrieving the thread id
#include <unistd.h>
#include <sys/syscall.h>
inline int get_tid( void )
{
  return syscall(SYS_gettid);
}

#include "worker.hpp"
#include "virtual_io.hpp"
#include "logger.hpp"
#include "global_view.hpp"


namespace margot
{
  // declare the prototype of the function that handles the incoming messages
  void handle_incoming_message(const message_t& new_message);

  // the function executed by the threadpool is quite generic
  void agora_worker_function( void )
  {
    // notify that we are a new thread
    margot::info("Thread ", get_tid(), " on duty");

    // assuming that there is plenty of work for everybody
    while (true)
    {

      // declaring the new message
      message_t new_incoming_message;

      if (!io::remote.recv_message(new_incoming_message))
      {
        margot::info("Thread ", get_tid(), " on retirement");
        return; // there is no more work available
      }

      // otherwise process the incoming message
      handle_incoming_message(new_incoming_message);
    }
  }

  // this is the actual function handles the incoming messages
  void handle_incoming_message(const message_t& new_message)
  {
    // the very first control is for system messages
    if (new_message.topic.compare("margot/system") == 0)
    {
      if (new_message.payload.compare("shutdown") ==  0)
      {
        io::remote.destroy_channel();
      }
    }

    // get the last part of the payload to understand what the message is about
    const auto start_type_pos = new_message.topic.find_last_of('/');
    const std::string message_type = new_message.topic.substr(start_type_pos);

    // ---------------------------------------------------------------------------------- handle the welcome message
    if (message_type.compare("/welcome") == 0)
    {
      // get the name of the application
      const auto application_name = new_message.topic.substr(7, start_type_pos - 7);

      // get the client id
      const auto client_id = new_message.payload;

      // get the application handler
      const auto application_handler = GlobalView::get_handler(application_name);

      // log the event
      pedantic("Thread ", get_tid(), ": new client \"", client_id, "\" for application \"", application_name);

      // handle the message
      application_handler->welcome_client(client_id);
    }

    // ---------------------------------------------------------------------------------- handle the kia message
    if (message_type.compare("/kia") == 0)
    {
      // get the name of the application
      const auto application_name = new_message.topic.substr(7, start_type_pos - 7);

      // get the client id
      const auto client_id = new_message.payload;

      // get the application handler
      const auto application_handler = GlobalView::get_handler(application_name);

      // log the event
      pedantic("Thread ", get_tid(), ": lost client \"", client_id, "\" for application \"", application_name);

      // handle the message
      application_handler->bye_client(client_id);
    }

    // ---------------------------------------------------------------------------------- handle the info message
    if (message_type.compare("/info") == 0)
    {
      // get the name of the application
      const auto application_name = new_message.topic.substr(7, start_type_pos - 7);

      // get the client id
      const auto application_info = new_message.payload;

      // get the application handler
      const auto application_handler = GlobalView::get_handler(application_name);

      // log the event
      pedantic("Thread ", get_tid(), ": received information about application \"", application_name, "\"", application_name);

      // handle the message
      application_handler->process_info(application_info);
    }

    // ---------------------------------------------------------------------------------- handle the observation message
    if (message_type.compare("/observation") == 0)
    {
      // get the name of the application
      const auto application_name = new_message.topic.substr(7, start_type_pos - 7);

      // get the client id
      const auto observation = new_message.payload;

      // get the application handler
      const auto application_handler = GlobalView::get_handler(application_name);

      // log the event
      pedantic("Thread ", get_tid(), ": received a new observation for \"", application_name, "\": \"", observation, "\"");

      // handle the message
      application_handler->new_observation(observation);
    }
  }
}
