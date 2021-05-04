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

#ifndef MARGOT_AGORA_MODEL_MESSAGE_HPP
#define MARGOT_AGORA_MODEL_MESSAGE_HPP

#include <cstring>
#include <string>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

namespace agora {

const std::string MESSAGE_HEADER = "margot";

// this is a generic message going through the agora virtual channel
struct message_model
{
  message_model() {}
  message_model(const std::string &topic, const std::string &payload) : topic(topic), payload(payload) {}

  std::string topic;
  std::string payload;
};

inline std::vector<std::string> tokenize(const std::string &message, const std::string &separator)
{
  std::vector<std::string> tokens;

  boost::split(tokens, message, boost::is_any_of(separator));

  return tokens;
}

} // namespace agora

#endif // MODEL_MESSAGE_HPP
