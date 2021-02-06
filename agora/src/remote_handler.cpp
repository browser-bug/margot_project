#include "agora/remote_handler.hpp"
#include "agora/application_manager.hpp"
#include "agora/paho_remote_handler.hpp"

using namespace agora;

RemoteHandler::RemoteHandler(const RemoteConfiguration &configuration) : configuration(configuration), inbox()
{
  ApplicationManager &am = ApplicationManager::get_instance();
  logger = am.get_logger();
}

std::unique_ptr<RemoteHandler> RemoteHandler::get_instance(const RemoteConfiguration &configuration)
{
  std::unique_ptr<RemoteHandler> remote_handler;

  switch (configuration.type)
  {
  case RemoteType::Paho:
    remote_handler = std::make_unique<PahoClient>(configuration);
    break;
  default:
    remote_handler = std::make_unique<PahoClient>(configuration);
  }

  return remote_handler;
}

void RemoteHandler::whitelist(message_model &incoming_string)
{
  static const std::string topic_accepted_characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_/^.";
  static const std::string payload_accepted_characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_ -.:,@<>=;()[]{}^*+'\"";

  const auto topic_invalid_idx = incoming_string.topic.find_first_not_of(topic_accepted_characters);
  const bool is_topic_invalid = topic_invalid_idx != std::string::npos;

  const auto payload_invalid_idx = incoming_string.payload.find_first_not_of(payload_accepted_characters);
  const bool is_payload_invalid = payload_invalid_idx != std::string::npos;

  if (is_topic_invalid || is_payload_invalid)
  {
    std::string error_msg = "Input sanitizer: found non valid characters. ";

    if (is_topic_invalid){
      error_msg += "-> ";
      error_msg += incoming_string.topic[topic_invalid_idx];
      error_msg +=  + " <- in the topic of the message.";
    }
    if (is_payload_invalid){
      error_msg += "-> ";
      error_msg += incoming_string.payload[payload_invalid_idx];
      error_msg += " <- in the payload of the message.";
    }

    incoming_string.topic = MESSAGE_HEADER + "/error/";
    incoming_string.payload = error_msg;
  }
}
