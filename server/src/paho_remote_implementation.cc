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
#include <cassert>
#include <unistd.h>      // to get the hostname
#include <sys/syscall.h> // i didn't tough that it was so difficult
#define gettid() syscall(SYS_gettid) // glibc wrapper missing


#include "agora/paho_remote_implementation.hpp"
#include "agora/logger.hpp"

using namespace agora;

#define MAX_HOSTNAME_LENGHT 256

extern "C"
{

  int recv_callback_function( void* recv_buffer, char* topic_c_str, int topic_size, MQTTClient_message* message )
  {

    // fix the string if it is broken and convert it to a std::string
    std::string payload((char*) message->payload, message->payloadlen);

    // log the reception of a message
    pedantic("MQTT callback: received a message on topic \"", topic_c_str, "\" with payload \"", payload, "\"");

    // compose the actual message
    struct message_t incoming_message = {std::string(topic_c_str), payload};

    // push the message in the queue
    // NOTE: this operation is dangerous, because we are assuming that context
    //       is actually refering to a safe queue in remote_handler.hpp. However,
    //       since we are dealing with a c interface... brace yourself!
    static_cast<RemoteHandler::MessageQueue*>(recv_buffer)->enqueue(incoming_message);

    // now we have to free the message memory
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topic_c_str);

    // everything is fine, we must return something different from 0
    // according to the documentation
    return 1;
  }

  void connlost_callback_function( void* recv_buffer, char* cause )
  {
    // first log that we are in trouble
    warning("MQTT callback: lost connection with broker due to \"", cause, "\"");

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
    pedantic("MQTT callback: succesfully delivered message with token \"", delivered_token, "\"");
  }


}


PahoClient::PahoClient( const std::string& application_name, const std::string& broker_address, const uint8_t qos_level, const std::string& username, const std::string& password )
  : RemoteHandler(), is_connected(false), qos_level(qos_level)
{
  // create the client id as unique-ish in the network
  char hostname[MAX_HOSTNAME_LENGHT];
  gethostname(hostname, MAX_HOSTNAME_LENGHT);
  client_id = std::string(hostname) + "_" + std::to_string(::gettid());

  // escape problematic characters
  std::replace(client_id.begin(), client_id.end(), '.', '_' );
  std::replace(client_id.begin(), client_id.end(), '-', '_' );

  // create the topic name for the last will and testment
  goodbye_topic = "margot/" + application_name + "/kia";

  // initialize the connection options
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  conn_opts.keepAliveInterval = 30;
  conn_opts.cleansession = 1;

  // set the last will and testment
  MQTTClient_willOptions last_will = MQTTClient_willOptions_initializer;
  last_will.topicName = goodbye_topic.c_str();
  last_will.message = client_id.c_str();
  last_will.qos = qos_level;
  conn_opts.will = &last_will;

  if (!username.empty())
  {
    conn_opts.username = username.c_str();
  }

  if (!password.empty())
  {
    conn_opts.password = password.c_str();
  }

  // initialize the client data structure
  int return_code = MQTTClient_create(&client, broker_address.c_str(), client_id.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);

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
    info("MQTT client: successfully connected to broker \"", broker_address, "\" as \"", username, "\"");
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
    warning(warning_string);
    throw std::runtime_error("MQTT client: unable to connect with broker due to \"" + error_cause + "\"");
  }

}


PahoClient::~PahoClient( void )
{
  // if we haven't done it yet, we have to disconnet the channel
  disconnect();
}


void PahoClient::send_message( const message_t&& output_message )
{
  // make sure to send a message while we are actually connected to a broker
  // it may happens in the shutdown procedure
  if (!is_connected)
  {
    warning("MQTT client: attempt to send a message while disconnected");
    return;
  }

  // this would be the "id" of the token to check in the log
  MQTTClient_deliveryToken delivery_token = 0;

  // compose the message using paho facilities
  MQTTClient_message message = MQTTClient_message_initializer;
  message.payload = (void*) output_message.payload.c_str(); //brrr, who should free the memory?
  message.payloadlen = output_message.payload.size();
  message.qos = qos_level;
  message.retained = 0;  // change to 1 if we want messages to appear to new subcribers

  // send the message (in asynchronous way) and it isn't thread safe >.>
  int return_code;
  {
    std::lock_guard<std::mutex> lock(send_mutex);
    return_code = MQTTClient_publishMessage(client, output_message.topic.c_str(), &message, &delivery_token);
  }

  // check if the message is actually sent in the outer space (no guarantees to delivery though)
  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to send a message, errno=" + std::to_string(return_code));
  }

  pedantic("MQTT client: sent message on topic \"", output_message.topic, "\" with token \"", delivery_token, "\"");
}


void PahoClient::subscribe( const std::string& topic )
{
  // make sure to subscibe while we are actually connected to a broker
  // it may happens in the shutdown procedure
  if (!is_connected)
  {
    warning("MQTT client: attempt to subscribe in a topic while disconnected");
    return;
  }

  // subscribe to the topic
  int return_code = MQTTClient_subscribe(client, topic.c_str(), qos_level);

  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to subscribe for topic \"" + topic + "\", errno=" + std::to_string(return_code));
  }

  pedantic("MQTT client: subscribed to topic \"", topic, "\"");
}


void PahoClient::unsubscribe( const std::string& topic )
{
  // make sure to unsubscibe while we are actually connected to a broker
  // it may happens in the shutdown procedure
  if (!is_connected)
  {
    warning("MQTT client: attempt to unsubscribe from a topic while disconnected");
    return;
  }

  int return_code = MQTTClient_unsubscribe(client, topic.c_str());

  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to unsubscribe to topic \"" + topic + "\", errno=" + std::to_string(return_code));
  }

  pedantic("MQTT client: unsubscribed to topic \"", topic, "\"");
}


void PahoClient::disconnect( void )
{
  // first of all we need to end the connection with the broker
  if (is_connected)
  {
    // send a goodbye message in the system
    send_message({goodbye_topic, client_id});

    // actually disconnect from the broker
    uint16_t disconnect_timeout_ms = 10000;
    warning("MQTT client: disconnecting from the broker (timeout ", disconnect_timeout_ms, "ms)");
    int return_code = MQTTClient_disconnect(client, disconnect_timeout_ms);

    // this is basically for show, since we cannot doing anything
    if (return_code != MQTTCLIENT_SUCCESS)
    {
      warning("MQTT client: unable to disconnect from client properly");
    }
    else
    {
      warning("MQTT client: we are now disconnected from the broker");
    }
  }

  // now we have to free the memory held by the client
  MQTTClient_destroy(&client);

  // state that we are no more in contact with the broker
  is_connected = false;

  // eventually we notify any worker in the inbox that we are out of business
  inbox.send_terminate_signal();
}


std::string PahoClient::get_my_client_id( void ) const
{
  return client_id;
}
