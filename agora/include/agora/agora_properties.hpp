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

#ifndef AGORA_PROPERTIES_HPP
#define AGORA_PROPERTIES_HPP

#include <string>
#include <unordered_map>
#include <filesystem>

namespace agora {

/**
 * @brief Available settings inside margot::heel configuration file.
 *
 * @details
 * These values are expected inside the 'agora' section of the Heel JSON configuration file.
 */
enum class AgoraSettingType {
    Invalid_Setting,         ///< Unexpected setting which will be ignored.
    Number_Config_Per_Iter,  ///< The number of configurations to explore each iteration.
    Number_Obs_Per_Config,   ///< The number of observation to expect from every configuration.
    Max_Number_Iter,         ///< The maximum number of iterations to perform during the iterative learning process.
    Doe_Plugin,              ///< The name of the plugin which performs the Design of Experiments.
    Clustering_Plugin,       ///< The name of the plugin which performs the clustering of input features.
    Storage_Type,            ///< The type of storage to use.
    Storage_Address,         ///< The storage address (e.g. a path for CSVs, a DB address for database storage, ...)
    Storage_Username,        ///< The storage username (ignored if not needed)
    Storage_Password         ///< The storage password (ignored if not needed)
};

/**
 * @brief Helper function used inside `switch` statements to resolve the setting type.
 *
 * @param [in] input The input string corresponding to the setting's name.
 *
 * @returns The setting type if found, Invalid_Setting otherwise.
 */
inline AgoraSettingType resolve_setting_type(const std::string &input) {
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
    if (itr != setting_types.end()) return itr->second;

    return AgoraSettingType::Invalid_Setting;
}

/**
 * @brief Available message types which Agora expects to receive.
 *
 * @details
 * These values are expected by the Agora message handler and are managed according to their type.
 */
enum class AgoraMessageType {
    Invalid_Message,  ///< Unexpected message which will be ignored.
    System,           ///< Used for internal operations.
    Welcome,          ///< Sent by new clients joining the pool.
    Kia,              ///< Sent by clients leaving the pool.
    Observation,      ///< A new observation corresponding to a specific configuration.
    Error             ///< Error message sent after a whitelist filtering.
};

/**
 * @brief Helper function used inside `switch` statements to resolve the message type.
 *
 * @param [in] input The input string corresponding to the message's type.
 *
 * @returns The message type if found, Invalid_Message otherwise.
 */
inline AgoraMessageType resolve_message_type(const std::string &input) {
    static const std::unordered_map<std::string, AgoraMessageType> message_types{
            {"system", AgoraMessageType::System},           {"welcome", AgoraMessageType::Welcome}, {"kia", AgoraMessageType::Kia},
            {"observation", AgoraMessageType::Observation}, {"error", AgoraMessageType::Error},
    };

    auto itr = message_types.find(input);
    if (itr != message_types.end()) return itr->second;

    return AgoraMessageType::Invalid_Message;
}

/**
 * @brief Available system commands which Agora expects to perform.
 *
 * @details
 * These values are expected by the Agora message handler after receiving a system message. Based on its type, different operations will be
 * performed.
 */
enum class AgoraSystemCommandType {
    Invalid_Command,  ///< Unexpected command which will be ignored.
    Shutdown,         ///< This tells Agora to shutdown all its operations.
    TestConnection    ///< This is used for testing purposes (e.g. testing the connection with a client)
};

/**
 * @brief Helper function used inside `switch` statements to resolve the system message command type.
 *
 * @param [in] input The input string corresponding to the system message's type.
 *
 * @returns The system message command type if found, Invalid_Command otherwise.
 */
inline AgoraSystemCommandType resolve_system_command_type(const std::string &input) {
    static const std::unordered_map<std::string, AgoraSystemCommandType> system_command_types{
            {"shutdown", AgoraSystemCommandType::Shutdown},
            {"test", AgoraSystemCommandType::TestConnection},
    };

    auto itr = system_command_types.find(input);
    if (itr != system_command_types.end()) return itr->second;

    return AgoraSystemCommandType::Invalid_Command;
}

/**
 * @brief The unique identifier of an application (AID).
 *
 * @details
 * This ID is used to represent a generic application inside Agora. The key is generated based on three elements:
 *  - Application name
 *  - Configuration file version
 *  - Block name
 */
struct application_id {
    application_id() {}
    /**
     * @brief Construct a new AID.
     *
     * @param [in] a_name The application name.
     * @param [in] v The configuration file version.
     * @param [in] b_name The block name.
     */
    application_id(const std::string &a_name, const std::string &v, const std::string &b_name)
            : app_name(a_name), version(v), block_name(b_name) {}

    std::string app_name;
    std::string version;
    std::string block_name;

    /// Check whether two AID are equal.
    inline bool operator==(const application_id &rhs) const {
        return app_name == rhs.app_name && version == rhs.version && block_name == rhs.block_name;
    }
    /// Convert the AID to string format.
    inline std::string str() const { return std::string(app_name + "^" + version + "^" + block_name); }
    /// Convert the AID to filesystem path format.
    inline std::filesystem::path path() const { return std::filesystem::path(app_name) / version / block_name; }
};

}  // namespace agora

#endif  // AGORA_PROPERTIES_HPP
