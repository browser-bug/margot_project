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

#ifndef MODEL_MESSAGE_HPP
#define MODEL_MESSAGE_HPP

#include <cstring>
#include <string>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

namespace agora {

/**
 * @brief The message header used by the server and the clients to exchange informations.
 */
const std::string MESSAGE_HEADER = "margot";

/**
 * @brief A data structure representing a generic message going through the Agora virtual channel.
 *
 * @details
 * A message is composed of a (topic, payload) pair.
 */
struct message_model {
    message_model() {}
    /**
     * @brief Construct a new message.
     *
     * @param [in] topic A string representing the message topic.
     * @param [in] payload A string representing the message payload.
     */
    message_model(const std::string &topic, const std::string &payload) : topic(topic), payload(payload) {}

    std::string topic;
    std::string payload;
};

/**
 * @brief Create a list of tokens by splitting a message using a separator character.
 *
 * @param [in] message A string representing the message to tokenize.
 * @param [in] separator A separator character which delimits the tokens.
 *
 * @returns A vector of tokens, each represented by a string.
 */
inline std::vector<std::string> tokenize(const std::string &message, const std::string &separator) {
    std::vector<std::string> tokens;

    boost::split(tokens, message, boost::is_any_of(separator));

    return tokens;
}

}  // namespace agora

#endif // MODEL_MESSAGE_HPP
