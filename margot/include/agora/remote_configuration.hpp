#ifndef REMOTE_CONFIGURATION_HPP
#define REMOTE_CONFIGURATION_HPP

#include <string>

namespace agora {

enum class RemoteType { Paho };

struct RemoteConfiguration
{
  RemoteConfiguration(RemoteType type = RemoteType::Paho) : type(type) {}

  inline void set_paho_handler_properties(const std::string &app_id, const std::string &broker_address, const uint8_t qos_level = 1,
                                          const std::string &username = "", const std::string &password = "",
                                          const std::string &broker_cert = "", const std::string &client_cert = "",
                                          const std::string &key = "")
  {
    app_identifier = app_id;
    qos = qos_level;
    broker_url = broker_address;
    broker_username = username;
    broker_password = password;
    broker_certificate = broker_cert;
    client_certificate = client_cert;
    client_key = key;
  }

  RemoteType type;

  // paho handler
  std::string app_identifier;
  uint8_t qos;
  std::string broker_url;
  std::string broker_username;
  std::string broker_password;
  std::string broker_certificate;
  std::string client_certificate;
  std::string client_key;
};

} // namespace agora

#endif // REMOTE_CONFIGURATION_HPP
