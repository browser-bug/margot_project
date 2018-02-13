/* agora/paho_remote_implementation.cc
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
#include <thread>
#include <atomic>
#include <stdexcept>

#include "paho_remote_implementation.hpp"
#include "logger.hpp"

using namespace margot;

extern "C"
{

  int recv_callback_function( void* recv_buffer, char* topic_c_str, int topic_size, MQTTClient_message* message )
  {
    // log the reception of a message
    margot::pedantic("MQTT callback: received a message on topic \"", topic_c_str, "\"");

    // get message and payload in a proper format
    struct message_t incoming_message = {std::string(topic_c_str), std::string((char*)message->payload)};

    // push the message in the queue
    // NOTE: this operation is dangerous, because we are assuming that context
    //       is actually refering to a safe queue in remote_handler.hpp. However,
    //       since we are dealing with a c interface... brace yourself!
    static_cast<RemoteHandler::MessageQueue*>(recv_buffer)->enqueue(incoming_message);

    // now we have to free the message memory
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topic_c_str);

    // not sure on why we should return a value, but still...
    return 1;
  }

  void connlost_callback_function( void* recv_buffer, char* cause )
  {
    // first log that we are in trouble
    margot::warning("MQTT callback: lost connection with broker due to \"", cause, "\"");

    // pretend that the error is a normal message
    struct message_t error_message = {"$disconnect$", std::string(cause)};

    // put the message in the inbox
    // NOTE: this operation is dangerous, because we are assuming that context
    //       is actually refering to a safe queue in remote_handler.hpp. However,
    //       since we are dealing with a c interface... brace yourself!
    static_cast<RemoteHandler::MessageQueue*>(recv_buffer)->enqueue(error_message);
  }

  void delivered_callback_function( void* context, MQTTClient_deliveryToken delivered_token)
  {
    // since we are not willing to explictly deal with tokens, we log this
    // event to be postprocessed externally.
    margot::pedantic("MQTT callback: succesfully delivered message with token \"", delivered_token, "\"");
  }


}


PahoClient::PahoClient( const std::string& broker_address, const uint8_t qos_level, const std::string& username, const std::string& password )
  : RemoteHandler(), is_connected(false), qos_level(qos_level), send_spinlock(ATOMIC_FLAG_INIT)
{
  // initialize the connection options
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  conn_opts.keepAliveInterval = 30;
  conn_opts.cleansession = 1;

  if (!username.empty())
  {
    conn_opts.username = username.c_str();
  }

  if (!password.empty())
  {
    conn_opts.password = password.c_str();
  }

  // initialize the client data structure
  int return_code = MQTTClient_create(&client, broker_address.c_str(), "margot-agora", MQTTCLIENT_PERSISTENCE_NONE, NULL);

  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to initialize client structure, errno=" + std::to_string(return_code));
  }

  // NOTE: if you change the second parameter of the following function, you will cause undefined
  //       behavior on all the callbacks, since they assume that the context is the address of the inbox
  return_code = MQTTClient_setCallbacks(client, static_cast<void*>(&inbox),
                                        connlost_callback_function,
                                        recv_callback_function,
                                        delivered_callback_function);

  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to set callbacks in client structure, errno=" + std::to_string(return_code));
  }

  // eventually estabilish the connection with the broker
  return_code = MQTTClient_connect(client, &conn_opts);

  // check the result
  if ( return_code == MQTTCLIENT_SUCCESS )
  {
    is_connected = true;
    margot::info("MQTT client: successfully connected to broker \"", broker_address, "\" as \"", username, "\"");
  }
  else
  {
    std::string error_cause;

    switch (return_code) // yeah i know, but documentation doesn't provide names
    {
      // for the connection errors, just numbers...
      case 1:
        error_cause = "Unacceptable protocol version";
        break;

      case 2:
        error_cause = "Identifier rejected";
        break;

      case 3:
        error_cause = "Server unavailable";
        break;

      case 4:
        error_cause = "Bad user name or password";
        break;

      case 5:
        error_cause = "Not authorized";
        break;

      default:
        error_cause = "Not reported in the documentation errno=" + std::to_string(return_code);
    }

    std::string warning_string = "MQTT client: unable to connect with broker \"" + broker_address + "\"";

    if (!username.empty())
    {
      warning_string += " as \"" + username + "\"";
    }

    warning_string += ", due to \"" + error_cause + "\"";
    margot::warning(warning_string);
    throw std::runtime_error("MQTT client: unable to connect with broker due to \"" + error_cause + "\"");
  }

}


PahoClient::~PahoClient( void )
{
  // first of all we need to end the connection with the broker and free the memory
  if (is_connected)
  {
    uint16_t disconnect_timeout_ms = 10000;
    margot::warning("MQTT client: disconnecting from the broker (timeout ", disconnect_timeout_ms, "ms)");
    int return_code = MQTTClient_disconnect(client, disconnect_timeout_ms);

    if (return_code != MQTTCLIENT_SUCCESS)
    {
      margot::warning("MQTT client: unable to disconnect from client properly");
    }
    else
    {
      margot::warning("MQTT client: we are now disconnected from the broker");
    }
  }

  MQTTClient_destroy(&client);

  // send the terminate signal in the queue, nice job everybody
  inbox.send_terminate_signal();
}


void PahoClient::send_message( message_t& output_message )
{
  // this would be the "id" of the token to check in the log
  MQTTClient_deliveryToken delivery_token;

  // compose the message using paho facilities
  MQTTClient_message message = MQTTClient_message_initializer;
  message.payload = (void*) output_message.payload.c_str(); //brrr, who should free the memory?
  message.payloadlen = output_message.payload.size();
  message.qos = qos_level;
  message.retained = 0;  // change to 1 if we want messages to appear to new subcribers

  // send the message (in asynchronous way) and it isn't thread safe >.>
  while (send_spinlock.test_and_set(std::memory_order_acquire)); // locking the sender

  int return_code = MQTTClient_publishMessage(client, output_message.topic.c_str(), &message, &delivery_token);
  send_spinlock.clear(std::memory_order_release); // releasing the sender

  // check if the message is actually sent in the outer space (no guarantees to delivery though)
  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to send a message, errno=" + std::to_string(return_code));
  }

  margot::pedantic("MQTT client: sent message on topic \"", output_message.topic, "\" with token \"", delivery_token, "\"");
}


void PahoClient::subscribe( const std::string& topic )
{
  int return_code = MQTTClient_subscribe(client, topic.c_str(), qos_level);

  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to subscribe for topic \"" + topic + "\", errno=" + std::to_string(return_code));
  }

  margot::pedantic("MQTT client: subscribed to topic \"", topic, "\"");
}


void PahoClient::unsubscribe( const std::string& topic )
{
  int return_code = MQTTClient_unsubscribe(client, topic.c_str());

  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to unsubscribe to topic \"" + topic + "\", errno=" + std::to_string(return_code));
  }

  margot::pedantic("MQTT client: unsubscribed to topic \"", topic, "\"");
}
