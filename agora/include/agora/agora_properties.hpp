#ifndef MARGOT_AGORA_TYPES_HPP
#define MARGOT_AGORA_TYPES_HPP

#include <string>
#include <unordered_map>
#include <filesystem>

// Possible types we can have inside agora for each object/concept

namespace agora {

enum class AgoraSettingType {
  Invalid_Setting,
  Number_Config_Per_Iter,
  Number_Obs_Per_Config,
  Max_Number_Iter,
  Doe_Plugin,
  Clustering_Plugin,
  Storage_Type,
  Storage_Address,
  Storage_Username,
  Storage_Password
};

// This helps switching on properties while parsing tables
inline AgoraSettingType resolve_setting_type(const std::string &input)
{
  static const std::unordered_map<std::string, AgoraSettingType> setting_types{
      {"number_configurations_per_iteration", AgoraSettingType::Number_Config_Per_Iter},
      {"number_observations_per_configuration", AgoraSettingType::Number_Obs_Per_Config},
      {"max_number_iteration", AgoraSettingType::Max_Number_Iter},
      {"storage_type", AgoraSettingType::Storage_Type},
      {"storage_address", AgoraSettingType::Storage_Address},
      {"storage_username", AgoraSettingType::Storage_Username},
      {"storage_password", AgoraSettingType::Storage_Password},
      {"doe_plugin", AgoraSettingType::Doe_Plugin},
      {"clustering_plugin", AgoraSettingType::Clustering_Plugin},
  };

  auto itr = setting_types.find(input);
  if (itr != setting_types.end())
    return itr->second;

  return AgoraSettingType::Invalid_Setting;
}

enum class AgoraMessageType { Invalid_Message, System, Welcome, Kia, Observation, Error };

// This helps switching on message types when receiving a new one
inline AgoraMessageType resolve_message_type(const std::string &input)
{
  static const std::unordered_map<std::string, AgoraMessageType> message_types{
      {"system", AgoraMessageType::System},           {"welcome", AgoraMessageType::Welcome}, {"kia", AgoraMessageType::Kia},
      {"observation", AgoraMessageType::Observation}, {"error", AgoraMessageType::Error},
  };

  auto itr = message_types.find(input);
  if (itr != message_types.end())
    return itr->second;

  return AgoraMessageType::Invalid_Message;
}

enum class AgoraSystemCommandType { Invalid_Command, Shutdown, TestConnection };

// This helps switching on system message types when receiving a new one
inline AgoraSystemCommandType resolve_system_command_type(const std::string &input)
{
  static const std::unordered_map<std::string, AgoraSystemCommandType> system_command_types{
      {"shutdown", AgoraSystemCommandType::Shutdown},
      {"test", AgoraSystemCommandType::TestConnection},
  };

  auto itr = system_command_types.find(input);
  if (itr != system_command_types.end())
    return itr->second;

  return AgoraSystemCommandType::Invalid_Command;
}

// Application Identifier
struct application_id
{
  application_id(const std::string &a_name, float v, const std::string &b_name) : app_name(a_name), version(v), block_name(b_name) {}

  std::string app_name;
  float version;
  std::string block_name;

  inline bool operator==(const application_id &rhs) const
  {
    return app_name == rhs.app_name && version == rhs.version && block_name == rhs.block_name;
  }
  inline std::string str() const { return std::string(app_name + " | " + std::to_string(version)+ " | " + block_name); }
  inline std::filesystem::path path() const { return std::filesystem::path(app_name) / std::to_string(version)/ block_name; }
};

} // namespace agora

#endif // MARGOT_TYPES_HPP
