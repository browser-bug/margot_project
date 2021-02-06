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
