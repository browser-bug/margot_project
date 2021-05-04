/* Agora library
 * Copyright (C) 2021 Bernardo Menicagli
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

#include <algorithm>
#include <atomic>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/syscall.h> // i didn't tough that it was so difficult
#include <thread>
#include <unistd.h> // to get the hostname

#include "agora/paho_remote_handler.hpp"
#include "agora/application_manager.hpp"

#define MAX_HOSTNAME_LENGHT 256

namespace agora {

extern "C" {

int recv_callback_function(void *recv_buffer, char *topic_c_str, int topic_size, MQTTClient_message *message)
{
  // NOTE: 	The length of the topic if there are one more NULL characters embedded in topicName, otherwise topicLen is 0. If topicLen is
  // 0, the value returned by strlen(topicName) can be trusted. If topicLen is greater than 0, the full topic name can be retrieved by
  // accessing topicName as a byte array of length topicLen.
  std::string topic(topic_c_str); // assuming topic_size is always 0
  std::string payload((char *)message->payload, message->payloadlen);

  // log the reception of a message
  ApplicationManager &am = ApplicationManager::get_instance();
  am.get_logger()->pedantic("MQTT callback: received a message on topic \"", topic_c_str, "\" with payload \"", payload, "\"");

  // compose the message using our struct wrapper
  message_model incoming_message(topic, payload);

  // push the message in the queue
  // NOTE: this operation is dangerous, because we are assuming that context
  //       is actually refering to a safe queue in remote_handler.hpp. However,
  //       since we are dealing with a c interface... brace yourself!
  static_cast<MessageQueue *>(recv_buffer)->enqueue(incoming_message);

  // now we have to free the message memory
  MQTTClient_freeMessage(&message);
  MQTTClient_free(topic_c_str);

  // everything is fine, we must return something different from 0
  // according to the documentation
  return 1;
}

void connlost_callback_function(void *recv_buffer, char *cause)
{
  // first log that we are in trouble
  ApplicationManager &am = ApplicationManager::get_instance();
  am.get_logger()->warning("MQTT callback: lost connection with broker due to ", cause);

  // pretend that the error is a normal message
  message_model error_message("$disconnect$", std::string(cause));

  // put the message in the inbox
  // NOTE: this operation is dangerous, because we are assuming that context
  //       is actually refering to a safe queue in remote_handler.hpp. However,
  //       since we are dealing with a c interface... brace yourself!
  static_cast<MessageQueue *>(recv_buffer)->enqueue(error_message);
}

void delivered_callback_function(void *context, MQTTClient_deliveryToken delivered_token)
{
  // since we are not willing to explictly deal with tokens, we log this event to be postprocessed externally.
  ApplicationManager &am = ApplicationManager::get_instance();
  am.get_logger()->pedantic("MQTT callback: succesfully delivered message with token \"", delivered_token, "\"");
}
}

PahoClient::PahoClient(const RemoteConfiguration& configuration)
    : RemoteHandler(configuration), is_connected(false), qos_level(configuration.qos)
{
  // create the client id as unique-ish in the network
  char hostname[MAX_HOSTNAME_LENGHT];
  gethostname(hostname, MAX_HOSTNAME_LENGHT);
  std::ostringstream ss;
  ss << std::this_thread::get_id();
  client_id = ss.str();

  // escape problematic characters
  std::replace(client_id.begin(), client_id.end(), '.', '_');
  std::replace(client_id.begin(), client_id.end(), '-', '_');

  // create the topic name for the last will and testment
  goodbye_topic = MESSAGE_HEADER + "/" + configuration.app_identifier + "/kia/" + client_id;

  // initialize the connection options
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  conn_opts.keepAliveInterval = 30;
  conn_opts.cleansession = 1;
  conn_opts.username = configuration.broker_username.empty() ? NULL : configuration.broker_username.c_str();
  conn_opts.password = configuration.broker_password.empty() ? NULL : configuration.broker_password.c_str();

  // initialize the connection with ssl certicates
  MQTTClient_SSLOptions ssl = MQTTClient_SSLOptions_initializer;

  if (!configuration.broker_certificate.empty())
  {
    ssl.enableServerCertAuth = true;
    ssl.trustStore = configuration.broker_certificate.c_str();

    ssl.keyStore = configuration.client_certificate.empty() ? NULL : configuration.client_certificate.c_str();
    ssl.privateKey = configuration.client_key.empty() ? NULL : configuration.client_key.c_str();

    conn_opts.ssl = &ssl;
  }

  logger->info("Broker address: ", configuration.broker_url.c_str());

  // set the last will and testment
  MQTTClient_willOptions last_will = MQTTClient_willOptions_initializer;
  last_will.topicName = goodbye_topic.c_str();
  last_will.message = "Client has been disconnected.";
  last_will.qos = qos_level;
  conn_opts.will = &last_will;

  // initialize the client data structure
  int return_code = MQTTClient_create(&client, configuration.broker_url.c_str(), client_id.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);

  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to initialize client structure, err=" + resolve_error_cause(return_code));
  }

  // NOTE: if you change the second parameter of the following function, you will cause undefined
  //       behavior on all the callbacks, since they assume that the context is the address of the inbox
  return_code = MQTTClient_setCallbacks(client, static_cast<void *>(&inbox), connlost_callback_function, recv_callback_function,
                                        delivered_callback_function);

  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to set callbacks in client structure, err=" + resolve_error_cause(return_code));
  }

  // eventually estabilish the connection with the broker
  return_code = MQTTClient_connect(client, &conn_opts);

  // check the result
  if (return_code == MQTTCLIENT_SUCCESS)
  {
    is_connected = true;
    logger->info("MQTT client: successfully connected to broker \"", configuration.broker_url, "\" as \"", configuration.broker_username,
                 "\"");
  } else
  {
    std::string error_cause = resolve_error_cause(return_code);

    logger->warning("MQTT client: unable to connect with broker \"", configuration.broker_url, "\", due to ", error_cause);
    throw std::runtime_error("MQTT client: unable to connect with broker due to \"" + error_cause + "\"");
  }
}

PahoClient::~PahoClient()
{
  // if we haven't done it yet, we have to disconnet the channel
  disconnect();
}

bool PahoClient::recv_message(message_model &input_message)
{
  const bool rc = inbox.dequeue(input_message);
  whitelist(input_message);
  return rc;
}

void PahoClient::send_message(const message_model &output_message)
{
  // make sure to send a message while we are actually connected to a broker
  // it may happens in the shutdown procedure
  if (!is_connected)
  {
    logger->warning("MQTT client: attempt to send a message while disconnected");
    return;
  }

  // this would be the "id" of the token to check in the log
  MQTTClient_deliveryToken delivery_token = 0;

  // compose the message using paho facilities
  MQTTClient_message message = MQTTClient_message_initializer;
  message.payload = (void *)output_message.payload.c_str();
  message.payloadlen = output_message.payload.size();
  message.qos = qos_level;
  message.retained = 0; // change to 1 if we want messages to appear to new subcribers

  // send the message (in asynchronous way) and it isn't thread safe >.>
  int return_code;
  {
    std::lock_guard<std::mutex> lock(send_mutex);
    return_code = MQTTClient_publishMessage(client, output_message.topic.c_str(), &message, &delivery_token);
  }

  // check if the message is actually sent in the outer space (no guarantees to delivery though)
  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to send a message, err=" + resolve_error_cause(return_code));
  }

  logger->pedantic("MQTT client: sent message on topic \"", output_message.topic, "\" with token ", delivery_token);
}

void PahoClient::subscribe(const std::string &topic)
{
  // make sure to subscibe while we are actually connected to a broker
  // it may happens in the shutdown procedure
  if (!is_connected)
  {
    logger->warning("MQTT client: attempt to subscribe in a topic while disconnected");
    return;
  }

  // subscribe to the topic
  int return_code = MQTTClient_subscribe(client, topic.c_str(), qos_level);

  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to subscribe for topic \"" + topic + "\", err=" + resolve_error_cause(return_code));
  }

  logger->pedantic("MQTT client: subscribed to topic \"", topic, "\"");
}

void PahoClient::unsubscribe(const std::string &topic)
{
  // make sure to unsubscibe while we are actually connected to a broker
  // it may happens in the shutdown procedure
  if (!is_connected)
  {
    logger->warning("MQTT client: attempt to unsubscribe from a topic while disconnected");
    return;
  }

  int return_code = MQTTClient_unsubscribe(client, topic.c_str());

  if (return_code != MQTTCLIENT_SUCCESS)
  {
    throw std::runtime_error("MQTT client: unable to unsubscribe from topic \"" + topic + "\", err=" + resolve_error_cause(return_code));
  }

  logger->pedantic("MQTT client: unsubscribed from topic \"", topic, "\"");
}

void PahoClient::disconnect()
{
  // first of all we need to end the connection with the broker
  if (is_connected)
  {
    // send a goodbye message in the system
    send_message({goodbye_topic, "Client has been disconnected."});

    // actually disconnect from the broker
    int disconnect_timeout_ms = 10000;
    logger->warning("MQTT client: disconnecting from the broker (timeout ", disconnect_timeout_ms, "ms)");
    int return_code = MQTTClient_disconnect(client, disconnect_timeout_ms);

    // this is basically for show, since we cannot doing anything
    if (return_code != MQTTCLIENT_SUCCESS)
      logger->warning("MQTT client: unable to disconnect from client properly, err=", resolve_error_cause(return_code));
    else
      logger->warning("MQTT client: we are now disconnected from the broker");
  }

  // now we have to free the memory held by the client
  MQTTClient_destroy(&client);

  // state that we are no more in contact with the broker
  is_connected = false;

  // eventually we notify any worker in the inbox that we are out of business
  inbox.send_terminate_signal();
}

std::string PahoClient::get_my_client_id() const { return client_id; }

std::string PahoClient::resolve_error_cause(int error_code) const
{
  // for the connection errors, we have just numbers...
  switch (error_code)
  {
  case 1:
    return "Unacceptable protocol version";
  case 2:
    return "Identifier rejected";
  case 3:
    return "Server unavailable";
  case 4:
    return "Bad user name or password";
  case 5:
    return "Not authorized";

  case -1:
    return "Generic error indicating failure of an MQTT client";
  case -2:
    return "MQTTCLIENT_PERSISTENCE_ERROR";
  case -3:
    return "MQTTCLIENT_DISCONNECTED";
  case -4:
    return "MQTTCLIENT_MAX_MESSAGES_INFLIGHT";
  case -5:
    return "MQTTCLIENT_BAD_UTF8_STRING";
  case -6:
    return "MQTTCLIENT_NULL_PARAMETER";
  case -7:
    return "MQTTCLIENT_TOPICNAME_TRUNCATED";
  case -8:
    return "MQTTCLIENT_BAD_STRUCTURE";
  case -9:
    return "MQTTCLIENT_BAD_QOS";
  case -10:
    return "MQTTCLIENT_SSL_NOT_SUPPORTED";
  case -11:
    return "MQTTCLIENT_BAD_MQTT_VERSION";
  case -14:
    return "MQTTCLIENT_BAD_PROTOCOL";
  case -15:
    return "MQTTCLIENT_BAD_MQTT_OPTION";
  case -16:
    return "MQTTCLIENT_WRONG_MQTT_VERSION";
  case -17:
    return "MQTTCLIENT_0_LEN_WILL_TOPIC";

  default:
    return "Not reported in the documentation errno=" + std::to_string(error_code);
  }
}

} // namespace agora
