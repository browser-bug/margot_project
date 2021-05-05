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

#ifndef MARGOT_AGORA_PAHO_REMOTE_IMPLEMENTATION_HPP
#define MARGOT_AGORA_PAHO_REMOTE_IMPLEMENTATION_HPP

#include <mutex>
#include <string>

extern "C" {
#include "MQTTClient.h"
}

#include "agora/remote_handler.hpp"

namespace agora {

class PahoClient : public RemoteHandler {
public:
    PahoClient(const RemoteConfiguration &configuration);
    ~PahoClient();

    // don't copy or move this object, things explode otherwise
    PahoClient(const PahoClient &) = delete;
    PahoClient(PahoClient &&) = delete;

    bool recv_message(message_model &input_message);

    void send_message(const message_model &output_message);

    void subscribe(const std::string &topic);

    void unsubscribe(const std::string &topic);

    std::string get_my_client_id() const;

    void disconnect();

private:
    MQTTClient client;
    bool is_connected;
    uint8_t qos_level;
    std::string client_id;
    std::mutex send_mutex;
    std::string goodbye_topic;

    std::string resolve_error_cause(int error_code) const;
};

}  // namespace agora

#endif  // MARGOT_AGORA_PAHO_REMOTE_IMPLEMENTATION_HDR
