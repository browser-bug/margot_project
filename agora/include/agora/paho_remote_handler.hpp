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

#ifndef PAHO_REMOTE_HANDLER_HPP
#define PAHO_REMOTE_HANDLER_HPP

#include <mutex>
#include <string>

extern "C" {
#include "MQTTClient.h"
}

#include "agora/remote_handler.hpp"

namespace agora {

/**
 * @brief Implementation of a RemoteHandler that leverages the Paho MQTT message protocol API.
 *
 * @details
 * The Paho MQTT message protocol is based on a Publish-Subscribe model. By using a series of topic, Agora is able to clearly distinguish
 * between different types of messages, automatically filtering out every other potential junk data. The available topics are:
 *  - welcome : to receive the application information once a new client connects.
 *  - observation : to receive the observations from the clients corresponding to a specific configuration.
 *  - kia : to receive kill/bye commands from a client.
 *  - system : to receive external system commands.
 *
 * @see [PahoMQTT C Library documentation](https://www.eclipse.org/paho/files/mqttdoc/MQTTClient/html/index.html) for more details.
 */
class PahoClient : public RemoteHandler {
public:
    /**
     * @brief Construct a new instance.
     *
     * @param [in] configuration The RemoteConfiguration to use.
     *
     * @details
     * This constructor performs the following actions in order:
     *  -# Create a new unique client ID for the Agora broker.
     *  -# Create a topic name for the last will and testament
     *    ([details](https://www.eclipse.org/paho/files/mqttdoc/MQTTClient/html/struct_m_q_t_t_client__will_options.html)).
     *  -# Initialize the connection options.
     *  -# Initialize the connection with SSL certificates (if any).
     *  -# Set the last will and testament (LWT).
     *  -# Initialize a new client data structure.
     *  -# Set the callbacks for asynchronous message reception
     *    ([details](https://www.eclipse.org/paho/files/mqttdoc/MQTTClient/html/async.html)).
     *  -# Finally, establish the connection with the Agora broker.
     */
    PahoClient(const RemoteConfiguration &configuration);
    /**
     * @brief Destruct the instance disconnecting from the channel.
     */
    ~PahoClient();

    // this class should not be copied or moved around
    PahoClient(const PahoClient &) = delete;
    PahoClient(PahoClient &&) = delete;

    /**
     * @brief Receive a new input message.
     *
     * @see RemoteHandler::recv_message()
     */
    bool recv_message(message_model &input_message);

    /**
     * @brief Send a new output message.
     *
     * @note
     * Since publishing is not thread-safe, a mutex is needed to ensure a synchronization between different threads.
     *
     * @see RemoteHandler::send_message()
     */
    void send_message(const message_model &output_message);

    /**
     * @brief Subscribe to a new topic.
     *
     * @see RemoteHandler::subscribe()
     */
    void subscribe(const std::string &topic);

    /**
     * @brief Unsubscribe to an existing topic.
     *
     * @see RemoteHandler::unsubscribe()
     */
    void unsubscribe(const std::string &topic);

    /**
     * @brief Terminate the Agora broker connection to the communication channel.
     *
     * @details
     * If still connected, sends a goodbye message to all the active clients.
     *
     * @see RemoteHandler::disconnect()
     */
    void disconnect();

    /**
     * @brief Get the unique identifier bound to the Agora broker.
     *
     * @see RemoteHandler::get_my_client_id()
     */
    std::string get_my_client_id() const;

private:
    /**
     * @brief A handle representing an MQTT client.
     */
    MQTTClient client;

    /**
     * @brief Store True if the Agora broker is connected, False otherwise.
     */
    bool is_connected;
    /**
     * @brief The Quality of Service for the communication channel.
     *
     * @details
     * The available QoS levels are:
     *  - Level 0: A message is delivered at most once and no acknowledgement of receiving is required.
     *  - Level 1: Every message is delivered at least once and a confirmation of receiving a message is required.
     *  - Level 2: A four-way handshake mechanism is used exactly once for the delivery of a message.
     */
    uint8_t qos_level;
    /**
     * @brief The client unique identifier.
     */
    std::string client_id;
    /**
     * @brief The goodbye topic name to use during disconnection.
     */
    std::string goodbye_topic;

    /**
     * @brief The mutex used to ensure a correct synchronization during message sending.
     */
    std::mutex send_mutex;

    /**
     * @brief Get the description of the specified error code.
     *
     * @param [in] error_code The error code to resolve.
     *
     * @returns A string representing the error code description.
     */
    std::string resolve_error_cause(int error_code) const;
};

}  // namespace agora

#endif // PAHO_REMOTE_HANDLER_HPP
