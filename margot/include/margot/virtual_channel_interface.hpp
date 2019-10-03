/* margot/virtual_channel_interface.hpp
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

#ifndef MARGOT_VIRTUAL_CHANNEL_INTERFACE
#define MARGOT_VIRTUAL_CHANNEL_INTERFACE

#include <string>
#include <memory>

namespace margot {


struct remote_message {
  std::string topic;
  std::string payload;
};

using remote_message_ptr = std::unique_ptr< remote_message >;


class VirtualChannelInterface {
 public:

  // this call has blocking semantic. It quits if disconnected, returning a null object
  virtual remote_message_ptr recv_message() = 0;

  virtual void send_message(remote_message_ptr& output_message) = 0;

  virtual void subscribe(const std::string& topic) = 0;

  virtual void unsubscribe(const std::string& topic) = 0;

  virtual void disconnect(void) = 0;

  virtual std::string get_my_client_id(void) const = 0;

  virtual ~VirtualChannelInterface(void) {}
};

}  // namespace margot

#endif  // MARGOT_VIRTUAL_CHANNEL_INTERFACE
