/* margot/paho_remote_implementation.cc
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

#include <sys/syscall.h>  // i didn't tough that it was so difficult
#include <unistd.h>       // to get the hostname
#include <atomic>
#include <cassert>
#include <stdexcept>
#include <string>
#include <thread>
#include <algorithm>
#define gettid() syscall(SYS_gettid)  // glibc wrapper missing

#include "margot/virtual_channel_impl_paho.hpp"

using namespace margot;

#define MAX_HOSTNAME_LENGHT 256


/******************************************************************
 *  CALLBACK FUNCTIONS CALLED BY THE MQTT FRAMEWORKS
 ******************************************************************/

extern "C" {

int recv_callback_function(void* context, char* topic_c_str, int topic_size,
                           MQTTClient_message* message) {
  // get the payload and the topic name of the message
  std::string payload((char*)message->payload, message->payloadlen);
  std::string topic(topic_c_str, topic_size);

  // enqueue the message in the related client
  // NOTE: this operation is dangerous, because we are assuming that context
  //       is actually refering to a valid Paho remote handler, but we can't be sure
  if (context != nullptr) {
    static_cast<PahoClient*>(context)->enqueue_message(topic, payload);
  }

  // now we have to free the message memory
  MQTTClient_freeMessage(&message);
  MQTTClient_free(topic_c_str);

  // everything is fine, we must return something different from 0
  // according to the documentation
  return 1;
}

// when this function is called, we got disconnected badly from the server
void connlost_callback_function(void* context, char* cause) {
  // put the message in the inbox
  // NOTE: this operation is dangerous, because we are assuming that context
  //       is actually refering to a valid Paho remote handler, but we can't be sure
  static_cast<PahoClient*>(context)->enqueue_message("$disconnect$", std::string(cause));
}

void delivered_callback_function(void* context, MQTTClient_deliveryToken delivered_token) {
  // since we are not willing to explictly deal with tokens, we don't do anything
}
}


/******************************************************************
 *  ACTUAL IMPLEMENTATION OF THE CLIENT
 ******************************************************************/

PahoClient::PahoClient(const std::string& application_name, const std::string& broker_address,
                       const uint8_t qos_level, const std::string& username, const std::string& password,
                       const std::string& trust_store, const std::string& client_certificate,
                       const std::string& client_key)
    : connection_status(connection_status_type::DISCONNECTED), qos_level(qos_level) {
  // create the client id as unique-ish in the network: <hostname>_<pid>
  char hostname[MAX_HOSTNAME_LENGHT];
  gethostname(hostname, MAX_HOSTNAME_LENGHT);
  client_id = std::string(hostname) + "_" + std::to_string(::gettid());

  // escape problematic characters
  std::replace(client_id.begin(), client_id.end(), '.', '_');
  std::replace(client_id.begin(), client_id.end(), '-', '_');

  // create the topic name for the last will and testment
  goodbye_topic = "margot/" + application_name + "/kia";

  // initialize the connection options
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  conn_opts.keepAliveInterval = 30;
  conn_opts.cleansession = 1;

  // initialize the connection with ssl certicates
  MQTTClient_SSLOptions ssl = MQTTClient_SSLOptions_initializer;
  if (!trust_store.empty()) {
    ssl.trustStore = trust_store.c_str();

    if (!client_certificate.empty()) {
      ssl.keyStore = client_certificate.c_str();
    }

    if (!client_key.empty()) {
      ssl.privateKey = client_key.c_str();
    }

    ssl.enableServerCertAuth = true;
    conn_opts.ssl = &ssl;
  }

  // set the last will and testment
  MQTTClient_willOptions last_will = MQTTClient_willOptions_initializer;
  last_will.topicName = goodbye_topic.c_str();
  last_will.message = client_id.c_str();
  last_will.qos = qos_level;
  conn_opts.will = &last_will;

  // set the username and password if they are not empty
  if (!username.empty()) {
    conn_opts.username = username.c_str();
  }
  if (!password.empty()) {
    conn_opts.password = password.c_str();
  }

  // initialize the client data structure
  int return_code = MQTTClient_create(&client, broker_address.c_str(), client_id.c_str(),
                                      MQTTCLIENT_PERSISTENCE_NONE, NULL);
  if (return_code != MQTTCLIENT_SUCCESS) {
    throw std::runtime_error("MQTT client: unable to initialize client structure, errno=" +
                             std::to_string(return_code));
  }

  // NOTE: if you change the second parameter of the following function, you will cause undefined
  //       behavior on all the callbacks, since they assume that the context is the address of this client
  return_code = MQTTClient_setCallbacks(client, static_cast<void*>(this), connlost_callback_function,
                                        recv_callback_function, delivered_callback_function);

  if (return_code != MQTTCLIENT_SUCCESS) {
    throw std::runtime_error("MQTT client: unable to set callbacks in client structure, errno=" +
                             std::to_string(return_code));
  }

  // eventually estabilish the connection with the broker
  return_code = MQTTClient_connect(client, &conn_opts);

  // check the result
  if (return_code == MQTTCLIENT_SUCCESS) {
    connection_status = connection_status_type::CONNECTED;
  } else {
    // something went bad
    std::string error_cause;
    switch (return_code)  // yeah i know, but documentation doesn't provide names
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
        error_cause = "Not reported in the documentation (usually no broker) errno=" + std::to_string(return_code);
    }
    std::string warning_string = "MQTT client: unable to connect with broker \"" + broker_address + "\"";

    if (!username.empty()) {
      warning_string += " as \"" + username + "\"";
    }

    warning_string += ", due to \"" + error_cause + "\"";
    throw std::runtime_error(warning_string);
  }
}

PahoClient::~PahoClient(void) {
  // if we haven't done it yet, we have to disconnet the channel
  disconnect();
}


int PahoClient::send(const std::string& topic, const std::string& payload) {
  // compose the message using paho facilities
  MQTTClient_deliveryToken delivery_token = 0; // this would be the "id" of the token
  MQTTClient_message message = MQTTClient_message_initializer;
  message.payload = (void*) payload.c_str();
  message.payloadlen = payload.size();
  message.qos = qos_level;
  message.retained = 0;  // change to 1 if we want messages to appear to new subcribers

  // send the message
  return MQTTClient_publishMessage(client, topic.c_str(), &message, &delivery_token);
}

void PahoClient::send_message(remote_message_ptr& output_message) {
  // send the message, one per client and only if we are connected
  assert(output_message != nullptr && "Error: attempting to send an empty message");
  int return_code;
  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (connection_status == connection_status_type::CONNECTED) {
      return_code = send(output_message->topic, output_message->payload);
    } else {
      // if we reach this statement, we are disconnected. Pretend that everything is fine
      return_code = MQTTCLIENT_SUCCESS;
    }
  }

  // check if the message is actually sent in the outer space (no guarantees to delivery though)
  if (return_code != MQTTCLIENT_SUCCESS) {
    throw std::runtime_error("MQTT client: unable to send a message, errno=" + std::to_string(return_code));
  } else {
    output_message.reset(); // free the memory of the message
  }
}

void PahoClient::subscribe(const std::string& topic) {
  // make sure to subscibe while we are actually connected to a broker
  // it may happens in the shutdown procedure
  std::lock_guard<std::mutex> lock(queue_mutex);
  if (connection_status == connection_status_type::CONNECTED) {
    const int return_code = MQTTClient_subscribe(client, topic.c_str(), qos_level);
    if (return_code != MQTTCLIENT_SUCCESS) {
      throw std::runtime_error("MQTT client: unable to subscribe for topic \"" + topic +
                               "\", errno=" + std::to_string(return_code));
    }
  }
}

void PahoClient::unsubscribe(const std::string& topic) {
  // make sure to unsubscibe while we are actually connected to a broker
  // it may happens in the shutdown procedure
  std::lock_guard<std::mutex> lock(queue_mutex);
  if (connection_status == connection_status_type::CONNECTED) {
    const int return_code = MQTTClient_unsubscribe(client, topic.c_str());
    if (return_code != MQTTCLIENT_SUCCESS) {
      throw std::runtime_error("MQTT client: unable to unsubscribe for topic \"" + topic +
                               "\", errno=" + std::to_string(return_code));
    }
  }
}

void PahoClient::disconnect(void) {
  // disconnect from the broker
  {
    std::lock_guard<std::mutex> lock(queue_mutex);
    if (connection_status == connection_status_type::CONNECTED) {
      send(goodbye_topic, client_id);
      const int disconnect_timeout_ms = 10000;
      MQTTClient_disconnect(client, disconnect_timeout_ms); // don't know what to do if it fails anyhow
      MQTTClient_destroy(&client);
      connection_status = connection_status_type::DISCONNECTED;
    }
  }
  // notify anyone waiting for a message
  recv_condition.notify_all();
}

std::string PahoClient::get_my_client_id(void) const { return client_id; }


remote_message_ptr PahoClient::recv_message() {
  // try to get the message from the queue (spourious events might happens!)
  std::unique_lock<std::mutex> lock(queue_mutex);
  while (connection_status == connection_status_type::CONNECTED && message_queue.empty()) {
    recv_condition.wait(lock);
  }

  // return the the message from the queue
  if (connection_status == connection_status_type::CONNECTED) {
    auto new_incoming_message = std::move(message_queue.back());
    message_queue.pop_back();
    return new_incoming_message;
  } else {
    // if we reach this statement, it means that we lost the connection badly
    return remote_message_ptr{};
  }
}


void PahoClient::enqueue_message(const std::string& topic, const std::string& payload) {
  {
    std::unique_lock<std::mutex> lock(queue_mutex);
    message_queue.emplace_front(remote_message_ptr( new remote_message{topic, payload}));
  }
  recv_condition.notify_one();
}