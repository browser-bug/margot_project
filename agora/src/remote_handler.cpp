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

#include "agora/remote_handler.hpp"
#include "agora/application_manager.hpp"
#include "agora/paho_remote_handler.hpp"

using namespace agora;

RemoteHandler::RemoteHandler(const RemoteConfiguration &configuration) : configuration(configuration), inbox() {
    ApplicationManager &am = ApplicationManager::get_instance();
    logger = am.get_logger();
}

std::unique_ptr<RemoteHandler> RemoteHandler::get_instance(const RemoteConfiguration &configuration) {
    std::unique_ptr<RemoteHandler> remote_handler;

    switch (configuration.type) {
        case RemoteType::Paho:
            remote_handler = std::make_unique<PahoClient>(configuration);
            break;
        default:
            remote_handler = std::make_unique<PahoClient>(configuration);
    }

    return remote_handler;
}

void RemoteHandler::whitelist(message_model &incoming_message) {
    static const std::string topic_accepted_characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_/^.";
    static const std::string payload_accepted_characters =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_ -.:,@<>=;()[]{}^*+'\"";

    const auto topic_invalid_idx = incoming_message.topic.find_first_not_of(topic_accepted_characters);
    const bool is_topic_invalid = topic_invalid_idx != std::string::npos;

    const auto payload_invalid_idx = incoming_message.payload.find_first_not_of(payload_accepted_characters);
    const bool is_payload_invalid = payload_invalid_idx != std::string::npos;

    if (is_topic_invalid || is_payload_invalid) {
        std::string error_msg = "Input sanitizer: found non valid characters. ";

        if (is_topic_invalid) {
            error_msg += "-> ";
            error_msg += incoming_message.topic[topic_invalid_idx];
            error_msg += +" <- in the topic of the message.";
        }
        if (is_payload_invalid) {
            error_msg += "-> ";
            error_msg += incoming_message.payload[payload_invalid_idx];
            error_msg += " <- in the payload of the message.";
        }

        incoming_message.topic = MESSAGE_HEADER + "/error/";
        incoming_message.payload = error_msg;
    }
}
