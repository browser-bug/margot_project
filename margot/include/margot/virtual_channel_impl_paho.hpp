/* margot/virtual_channel_impl_paho.hpp
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

#ifndef MARGOT_VIRTUAL_CHANNEL_IMPL_PAHO_HDR
#define MARGOT_VIRTUAL_CHANNEL_IMPL_PAHO_HDR

#include <mutex>
#include <string>
#include <condition_variable>
#include <deque>

extern "C" {
#include "MQTTClient.h"
}

#include "margot/virtual_channel_interface.hpp"

namespace margot {

class PahoClient : public VirtualChannelInterface {

  enum class connection_status_type {
    DISCONNECTED,
    CONNECTED
  };

  MQTTClient client;
  connection_status_type connection_status;
  uint8_t qos_level;
  std::string client_id;
  std::string goodbye_topic;
  std::mutex queue_mutex;
  std::condition_variable recv_condition;
  std::deque< remote_message_ptr > message_queue;

  // this function just send a message through MQTT, without locking
  int send(const std::string& topic, const std::string& payload);

 public:
  PahoClient(const std::string& application_name, const std::string& broker_address,
             const uint8_t qos_level = 1, const std::string& username = "", const std::string& password = "",
             const std::string& trust_store = "", const std::string& client_certificate = "",
             const std::string& client_key = "");
  ~PahoClient(void);

  // don't copy or move this object, things will explode otherwise
  PahoClient(const PahoClient&) = delete;
  PahoClient(PahoClient&&) = delete;

  // these are the messages that implemets the virtual channel interface
  remote_message_ptr recv_message();
  void send_message(remote_message_ptr& output_message);
  void subscribe(const std::string& topic);
  void unsubscribe(const std::string& topic);
  std::string get_my_client_id(void) const;
  void disconnect(void);

  // this method is used by the paho callback functions to enqueue a new message
  void enqueue_message(const std::string& topic, const std::string& payload);
};

}  // namespace margot

#endif  // MARGOT_VIRTUAL_CHANNEL_IMPL_PAHO_HDR
