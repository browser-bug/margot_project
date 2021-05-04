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

#ifndef MARGOT_AGORA_REMOTE_HANDLER_HDR
#define MARGOT_AGORA_REMOTE_HANDLER_HDR

#include <string>

#include "agora/logger.hpp"
#include "agora/model_message.hpp"
#include "agora/remote_configuration.hpp"
#include "agora/safe_queue.hpp"

namespace agora {

using MessageQueue = Queue<message_model>;

class RemoteHandler {
public:
  static std::unique_ptr<RemoteHandler> get_instance(const RemoteConfiguration &configuration);

  virtual ~RemoteHandler() {};

  // this class should not be copied or moved around
  RemoteHandler(const RemoteHandler &) = delete;
  RemoteHandler(RemoteHandler &&) = delete;

  virtual bool recv_message(message_model &input_message) = 0;

  virtual void send_message(const message_model &output_message) = 0;

  virtual void subscribe(const std::string &topic) = 0;

  virtual void unsubscribe(const std::string &topic) = 0;

  virtual void disconnect() = 0;

  virtual std::string get_my_client_id() const = 0;

protected:
  RemoteHandler(const RemoteConfiguration &configuration);
  RemoteConfiguration configuration;

  MessageQueue inbox;
  std::shared_ptr<Logger> logger;

  void whitelist(message_model &incoming_string);
};

} // namespace agora

#endif // MARGOT_AGORA_REMOTE_HANDLER_HDR
