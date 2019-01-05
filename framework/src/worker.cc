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

#include <string>
#include <iostream>

// those two headers are retrieving the thread id
#include <unistd.h>
#include <sys/syscall.h>
inline int get_tid( void )
{
  return syscall(SYS_gettid);
}

#include "agora/worker.hpp"
#include "agora/virtual_io.hpp"
#include "agora/logger.hpp"
#include "agora/global_view.hpp"


namespace agora
{
  // declare the prototype of the function that handles the incoming messages
  void handle_incoming_message(const message_t& new_message);

  // the function executed by the threadpool is quite generic
  void agora_worker_function( void )
  {
    // notify that we are a new thread
    info("Thread ", get_tid(), " on duty");

    // assuming that there is plenty of work for everybody
    while (true)
    {

      // declaring the new message
      message_t new_incoming_message;

      if (!io::remote.recv_message(new_incoming_message))
      {
        info("Thread ", get_tid(), " on retirement");
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

    // ---------------------------------------------------------------------------------- handle the welcome message from the clients/beholder
    if (message_type.compare("/welcome") == 0)
    {
      // I need to differentiate the sender of the message:
      // if the sender is the beholder: beholder/welcome
      if (std::count(new_message.topic.begin(), new_message.topic.end(), '/') == 1)
      {
        // log the event
        info("Thread ", get_tid(), ": Received beholder welcome message.\nSending messages to beholder of the applications for which agorà has a model...");

        // io::remote.send_message({"beholder/status", ""});

        // log the event
        // pedantic("Thread ", get_tid(), ": status message sent to beholder to prove agorà's vitality.\nSending messages to beholder of the applications for which agorà has a model...");

        // get the list of applications which currently have the model and whose clients' list is not empty
        const auto app_list = GlobalView::get_handlers_with_model();

        io::remote.send_message({"margot/agora/beholder/welcome", ""});
        pedantic("Thread ", get_tid(), ": agorà has no applications with model currently. Sending welcome message to beholder to aknowledge agorà's vitality.'");

        if (app_list.size() != 0)
        {
          for (auto& i : app_list)
          {
            io::remote.send_message({"beholder/" + i + "/model", ""});
            pedantic("Thread ", get_tid(), ": model message sent to beholder to inform that there is a model for application: ", i);
          }
        }
      }
      else
      {
        // if the sender is a client: margot/+/+/+/welcome
        // get the name of the application
        const auto application_name = new_message.topic.substr(7, start_type_pos - 7);

        // get the client id
        const auto client_id = new_message.payload;

        // get the application handler
        const auto application_handler = GlobalView::get_handler(application_name);

        // log the event
        pedantic("Thread ", get_tid(), ": new client \"", client_id, "\" for application \"", application_name);

        // handle the message
        application_handler->welcome_client(client_id, application_name);
      }
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

    // ---------------------------------------------------------------------------------- handle the application-specific beholder commands
    if (message_type.compare("/commands") == 0)
    {
      // log the event
      info("Thread ", get_tid(), ": Received beholder command");

      // get the name of the application
      const auto application_name = new_message.topic.substr(6, start_type_pos - 6);

      // get the client id
      const auto observation = new_message.payload;

      // look for a space in the payload
      // we need to differentiate the case in which we are given the timestamp of the last observation
      // to just delete the trace from its top to this element or if we are not given anything
      // and we will delete the whole trace.
      const auto pos_first_space = observation.find_first_of(' ', 0);

      // just a check that the unique command (as of now) is the correct word "retraining"
      if (observation.substr(0, pos_first_space).compare("retraining") == 0)
      {
        // get the application handler
        const auto application_handler = GlobalView::get_handler(application_name);

        // log the event
        pedantic("Thread ", get_tid(), ": received retraining command for application: \"", application_name);

        // handle the message
        if (pos_first_space != std::string::npos)
        {
          // if actually there is the space then pass the whole rest of the payload containing the timestamps
          application_handler->retraining(observation.substr(pos_first_space + 1, std::string::npos));
        }
        else
        {
          // if there is no space then pass the "null" string
          application_handler->retraining("null");
        }
      }

    }
  }
}
