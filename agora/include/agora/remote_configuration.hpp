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

#ifndef REMOTE_CONFIGURATION_HPP
#define REMOTE_CONFIGURATION_HPP

#include <string>

namespace agora {

/**
 * @brief Available RemoteHandler implementations.
 *
 * @details
 * These values represents a list of available message handler implementations which specifies a generic RemoteHandler interface.
 */
enum class RemoteType { Paho };

/**
 * @brief A generic configuration for a RemoteHandler.
 *
 * @details
 * This data structure contains the specification for a generic remote message handler. This includes:
 *  - The broker address (i.e. URL) which is used to establish a connection by the clients.
 *  - The quality of service for the communication channel.
 *  - The broker username (if any security level is present).
 *  - The broker password (if any security level is present).
 *  - The broker certificate in case of SSL connections.
 *  - The client certificate in case of SSL connections.
 *  - The client key in case of SSL connections.
 */
struct RemoteConfiguration {
    /**
     * @brief Construct a new RemoteHandler configuration.
     *
     * @param [in] type The type of implementation to use.
     */
    RemoteConfiguration(RemoteType type = RemoteType::Paho) : type(type) {}

    /**
     * @brief Set the properties for the Paho MQTT protocol.
     *
     * @param [in] app_id The AID corresponding to the application.
     * @param [in] broker_address The broker address to use.
     * @param [in] qos_level The quality of service of the channel.
     * @param [in] username The broker username to use.
     * @param [in] password The broker password to use.
     * @param [in] broker_cert The broker certificate to use.
     * @param [in] client_cert The client certificate to use.
     * @param [in] key The client key to use.
     */
    void set_paho_handler_properties(const std::string &app_id, const std::string &broker_address, const uint8_t qos_level = 1,
                                            const std::string &username = "", const std::string &password = "",
                                            const std::string &broker_cert = "", const std::string &client_cert = "",
                                            const std::string &key = "") {
        app_identifier = app_id;
        broker_url = broker_address;
        qos = qos_level;
        broker_username = username;
        broker_password = password;
        broker_certificate = broker_cert;
        client_certificate = client_cert;
        client_key = key;
    }

    /// The implementation type to use.
    RemoteType type;

    // Paho MQTT handler configuration properties.
    std::string app_identifier;
    std::string broker_url;
    uint8_t qos;
    std::string broker_username;
    std::string broker_password;
    std::string broker_certificate;
    std::string client_certificate;
    std::string client_key;
};

}  // namespace agora

#endif // REMOTE_CONFIGURATION_HPP
