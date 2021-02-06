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
  PahoClient(const RemoteConfiguration& configuration);
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

} // namespace agora

#endif // MARGOT_AGORA_PAHO_REMOTE_IMPLEMENTATION_HDR
