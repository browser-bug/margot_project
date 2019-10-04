/* margot/virtual_channel.hpp
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

#ifndef MARGOT_VIRTUAL_CHANNEL_HDR
#define MARGOT_VIRTUAL_CHANNEL_HDR

#include <cassert>
#include <memory>

#include "margot/virtual_channel_interface.hpp"

namespace margot {

class VirtualChannel {
 private:
  std::unique_ptr<VirtualChannelInterface> channel;

 public:
  template <class T, class... Ts>
  inline void create(const Ts&... remote_arguments) {
    channel.reset(new T(remote_arguments...));
  }

  inline void destroy_channel(void) {
    assert(channel && "Error: destroy on an empty channel");
    channel->disconnect();
  }

  inline remote_message_ptr recv_message(void) {
    assert(channel && "Error: recv on an empty channel");
    return channel->recv_message();
  }

  inline void send_message(remote_message_ptr& output_message) {
    assert(channel && "Error: send on an empty channel");
    channel->send_message(output_message);
  }

  inline void subscribe(const std::string& topic) {
    assert(channel && "Error: subscribe on an empty channel");
    channel->subscribe(topic);
  }

  inline void unsubscribe(const std::string& topic) {
    assert(channel && "Error: unsubscribe on an empty channel");
    channel->subscribe(topic);
  }

  inline std::string get_my_client_id(void) const {
    assert(channel && "Error: unable to get the client id from an empty channel");
    return channel->get_my_client_id();
  }
};
}  // namespace margot

#endif  // MARGOT_VIRTUAL_CHANNEL_HDR
