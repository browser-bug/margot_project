/* beholder/worker_beholder.cc
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

#include <string>
#include <iostream>

// those two headers are retrieving the thread id
#include <unistd.h>
#include <sys/syscall.h>
inline int get_tid( void )
{
  return syscall(SYS_gettid);
}

#include "beholder/worker_beholder.hpp"
#include "agora/virtual_io.hpp"
#include "agora/logger.hpp"
#include "beholder/global_view_beholder.hpp"


namespace beholder
{

  using message_t = agora::message_t;

  // declare the prototype of the function that handles the incoming messages
  void handle_incoming_message(const message_t& new_message);

  // the function executed by the threadpool is quite generic
  void beholder_worker_function( void )
  {
    // notify that we are a new thread
    agora::info("Thread ", get_tid(), " on duty");

    // assuming that there is plenty of work for everybody
    while (true)
    {

      // declaring the new message
      message_t new_incoming_message;

      if (!agora::io::remote.recv_message(new_incoming_message))
      {
        agora::info("Thread ", get_tid(), " on retirement");
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
        agora::io::remote.destroy_channel();
      }
    }

    // get the last part of the payload to understand what the message is about
    const auto start_type_pos = new_message.topic.find_last_of('/');
    const std::string message_type = new_message.topic.substr(start_type_pos);

    // ---------------------------------------------------------------------------------- handle the status message from agorà
    if (message_type.compare("/status") == 0)
    {
      // enable the beholder
      GlobalView::set_with_agora_true();

      // log the event
      agora::pedantic("Thread ", get_tid(), ": agorà is alive and kicking! Beholder ENABLED.\nStarting sync with agorà of the applications for which there is a model...");

      // // get the list of applications
      // const auto observation = new_message.payload;
      //
      // // if no application_handler of agorà has the model and active clients
      // if (observation.length() == 0)
      // {
      //   // log the event
      //   agora::pedantic("Thread ", get_tid(), ": received status summary. Currently no application is managed by the beholder.");
      //
      //   return;
      // }
      //
      // // log the event
      // agora::pedantic("Thread ", get_tid(), ": received current status summary from agorà.");
      //
      // // vector to store the applications' names
      // std::vector<std::string> temp_list;
      //
      // // to parsee the payload
      // std::stringstream ss(observation);
      //
      // // parse the incoming message and split it while filling the vector of applications' names
      // while (ss.good())
      // {
      //   std::string substr;
      //   getline( ss, substr, '@' );
      //   temp_list.push_back(substr);
      // }
      //
      // for (auto& i : temp_list)
      // {
      //   // get the application handler
      //   const auto application_handler = GlobalView::get_handler(i);
      //
      //   // log the event
      //   agora::pedantic("Thread ", get_tid(), ": new beholder handler created from status for application \"", i);
      // }
    }

    // ---------------------------------------------------------------------------------- handle the creation of new handlers at runtime (not from status summary)
    if (message_type.compare("/model") == 0)
    {
      // get the name of the application
      const std::string application_name = new_message.topic.substr(7, start_type_pos - 7);

      // bool to understand whether we are receiving a model from a broadcast message
      // or a messagge specifically addressed to the beholder
      bool broadcast_model;

      // count the letters on the first level topic to distinguish between the topic:
      // "margot/" vs "beholder/"
      auto num_chars = new_message.topic.find_first_of("/", 0);

      // to understand which type of message we are dealing with,
      // it can be either of type:
      // "margot/+/+/+/model" = model broadcast
      // or
      // "beholder/+/+/+/model" = message sent by agora specifically for the beholder to
      // inform the beholder of the applications for which currently agorà already has a model
      // and for which the beholder has to prepare the handler and accept observations
      // if "margot/..."
      if (num_chars == 6)
      {
        // log the event
        agora::pedantic("Thread ", get_tid(), ": received broadcast message of a (brand new) model from agorà for application: ", application_name);
        broadcast_model = true;
      }
      // if "beholder/..."
      else
      {
        // log the event
        agora::pedantic("Thread ", get_tid(), ": received a startup sync message from agorà to inform that a model is available for application: ", application_name);
        broadcast_model = false;
      }

      // before getting the handler (otherwise the handler gets constructed anyways...)
      // we want to know if this application is already managed by the beholder
      const bool handler_already_present = GlobalView::is_managing(application_name);

      // get the application handler
      const auto application_handler = GlobalView::get_handler(application_name);

      // in this way we only re-enable the handler if:
      // 1) it was already present
      // (otherwise it is useless if it is a new one the constructor sets the status to READY already)
      // 2) if it is a model broadcast message, so after model recomputation,
      // we do not re-enable the handler if it is just a client model message.
      // If the following method is called in other situations it is theoretically painless
      // because it only sets the handler to READY executed when we receive a broadcast model
      // of and application which is already being managed by the beholder
      // and it is currently in the DISABLED status
      // (if it is in the COMPUTING status it will not do anything)
      if (broadcast_model && handler_already_present)
      {
        // set the handler status to READY to receive observations
        application_handler->set_handler_ready();
      }

      // log the event
      agora::pedantic("Thread ", get_tid(), ": new beholder handler for application \"", application_name);
    }

    // ---------------------------------------------------------------------------------- handle the observation message
    if (message_type.compare("/observation") == 0)
    {
      // get the name of the application
      const auto application_name = new_message.topic.substr(9, start_type_pos - 9);

      // check if agora is online
      if (!(GlobalView::is_with_agora()))
      {
        // log the event
        agora::pedantic("Thread ", get_tid(), ": Agorà offline - DISCARDING the received observation for \"", application_name);
        return;
      }

      // if we already have an handler for the application then we consider the message, otherwise we discard it
      // otherwise we risk parsing observation messages for applications which do not have a model yet
      if (!(GlobalView::is_managing(application_name)))
      {
        agora::pedantic("Discarding observation: we don't have a model yet for: ", application_name);
        return;
      }

      // get the client id
      const auto observation = new_message.payload;

      // get the application handler
      const auto application_handler = GlobalView::get_handler(application_name);

      // log the event
      agora::pedantic("Thread ", get_tid(), ": received a new observation for \"", application_name, "\": \"", observation, "\"");

      // handle the message
      application_handler->new_observation(observation);
    }

    // ---------------------------------------------------------------------------------- handle the kia message from agorà/client
    if (message_type.compare("/kia") == 0)
    {
      // if there are just two slashes in the topic we are receiving the kia from agora
      if (std::count(new_message.topic.begin(), new_message.topic.end(), '/') == 2)
      {
        // log the event
        agora::pedantic("Thread ", get_tid(), ": received kia message from agorà.");

        // temporary disable the beholder
        GlobalView::set_with_agora_false();

        // log the event
        agora::info("Thread ", get_tid(), ": all the beholder's handlers have been stopped following agora's departure. Waiting for agora's resurrection.");
      }
      else
      {
        // we are receiving the kia message from a client

        // get the name of the application
        const auto application_name = new_message.topic.substr(7, start_type_pos - 7);

        // get the client id
        const auto client_id = new_message.payload;

        // if we are managing the application we get the corresponding client
        // and delete this client from the list of managed clients
        if (GlobalView::is_managing(application_name))
        {
          // get the application handler
          const auto application_handler = GlobalView::get_handler(application_name);

          // handle the message
          application_handler->bye_client(client_id);

          // log the event
          agora::pedantic("Thread ", get_tid(), ": received kia message from client: ", client_id, " for application: ", application_name, ". Removing client from the list of managed clients.");
        }
        else
        {
          // log the event
          agora::debug("Thread ", get_tid(), ": received kia message from client: ", client_id, " for application: ", application_name,
                       ". Discarding the message since the beholder is not currently managing that application.");
        }
      }
    }

    if (message_type.compare("/welcome") == 0)
    {
      // log the event
      agora::pedantic("Thread ", get_tid(), ": received welcome message from agorà.");

      // enable the beholder
      GlobalView::set_with_agora_true();

      // log the event
      //agora::info("Thread ", get_tid(), ": all the beholder's handlers (if any) have been restarted following agora's resurrection.");

      // log the event
      agora::info("Thread ", get_tid(), ": requesting current status to agorà.");

      // sends the request of summary of current status to agorà
      agora::io::remote.send_message({"agora/status", "Beholder requesting current agora status"});

    }

  }
}
