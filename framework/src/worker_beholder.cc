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
      agora::pedantic("Thread ", get_tid(), ": agorà is alive and kicking! Beholder ENABLED.");

      // get the list of applications
      const auto observation = new_message.payload;

      // if no application_handler of agorà has the model and active clients
      if (observation.length() == 0)
      {
        // log the event
        agora::pedantic("Thread ", get_tid(), ": received status summary. Currently no application is managed by the beholder.");

        return;
      }

      // log the event
      agora::pedantic("Thread ", get_tid(), ": received current status summary from agorà.");

      // vector to store the applications' names
      std::vector<std::string> temp_list;

      // to parsee the payload
      std::stringstream ss(observation);

      // parse the incoming message and split it while filling the vector of applications' names
      while (ss.good())
      {
        std::string substr;
        getline( ss, substr, '@' );
        temp_list.push_back(substr);
      }

      for (auto& i : temp_list)
      {
        // get the application handler
        const auto application_handler = GlobalView::get_handler(i);

        // log the event
        agora::pedantic("Thread ", get_tid(), ": new beholder handler created from status for application \"", i);
      }
    }

    // ---------------------------------------------------------------------------------- handle the creation of new handlers at runtime (not from status summary)
    if (message_type.compare("/model") == 0)
    {


      // count the number of "/" to understand which type of message we are dealing with,
      // it can be either of type:
      // "margot/+/+/+/model"
      // or
      // "margot/+/+/+/+/model", where the last "+" wildcard represents the client name
      // that we need to remove to uniform the two versions of the "/model" message
      std::string application_name;

      // we are in the case of the topic with four "+" wildcards, i.e five "/"
      if (std::count(new_message.topic.begin(), new_message.topic.end(), '/') == 5)
      {
        // create a substring of the topic without the last topic level
        std::string substring = new_message.topic.substr(0, start_type_pos);

        //repeat the process to find the position of the "new" last "/"
        const auto start_type_pos_substring = substring.find_last_of('/');

        // get the name of the application
        application_name = new_message.topic.substr(7, start_type_pos_substring - 7);
      }
      // we are in the case of the topic with three "+" wildcards
      else
      {
        // get the name of the application
        application_name = new_message.topic.substr(7, start_type_pos - 7);
      }

      // get the application handler
      const auto application_handler = GlobalView::get_handler(application_name);

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

    // ---------------------------------------------------------------------------------- handle the kia message from agorà
    if (message_type.compare("/kia") == 0)
    {
      // log the event
      agora::pedantic("Thread ", get_tid(), ": received kia message from agorà.");

      // temporary disable the beholder
      GlobalView::set_with_agora_false();

      // log the event
      agora::info("Thread ", get_tid(), ": all the beholder's handlers have been stopped following agora's departure. Waiting for agora's resurrection.");
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
