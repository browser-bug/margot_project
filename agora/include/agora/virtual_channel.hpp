#ifndef MARGOT_AGORA_VIRTUAL_CHANNEL_HPP
#define MARGOT_AGORA_VIRTUAL_CHANNEL_HPP

#include <cassert>
#include <memory>

#include "agora/logger.hpp"
#include "agora/remote_handler.hpp"

namespace agora {

class VirtualChannel {
private:
  std::shared_ptr<RemoteHandler> channel;

public:
  inline void create(const RemoteConfiguration &configuration, std::shared_ptr<Logger> logger) {
    channel = RemoteHandler::get_instance(configuration);
  }

  inline void destroy_channel()
  {
    assert(channel && "Error: destroy on an empty channel");
    channel->disconnect();
  }

  inline bool recv_message(message_model &input_message)
  {
    assert(channel && "Error: recv on an empty channel");
    return channel->recv_message(input_message);
  }

  inline void send_message(const message_model &&output_message)
  {
    assert(channel && "Error: send on an empty channel");
    channel->send_message(std::move(output_message));
  }

  inline void subscribe(const std::string &topic)
  {
    assert(channel && "Error: subscribe on an empty channel");
    channel->subscribe(topic);
  }

  inline void unsubscribe(const std::string &topic)
  {
    assert(channel && "Error: unsubscribe on an empty channel");
    channel->subscribe(topic);
  }

  inline std::string get_my_client_id() const
  {
    assert(channel && "Error: unable to get the client id from an empty channel");
    return channel->get_my_client_id();
  }
};
} // namespace agora

#endif // MARGOT_AGORA_VIRTUAL_CHANNEL_HPP
