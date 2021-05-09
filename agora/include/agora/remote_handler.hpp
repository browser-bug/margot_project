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

#ifndef REMOTE_HANDLER_HPP
#define REMOTE_HANDLER_HPP

#include <string>

#include "agora/logger.hpp"
#include "agora/model_message.hpp"
#include "agora/remote_configuration.hpp"
#include "agora/safe_queue.hpp"

namespace agora {

/**
 * @brief A synchronized message queue in which the messages are stored.
 */
using MessageQueue = Queue<message_model>;

/**
 * @brief Interface representing a generic remote message handler entity.
 *
 * @details
 * This interface implements a factory pattern which enables the user to get a new instance of the class deriving from it depending on the
 * provided configuration. Provide a public API containing the methods to manage the communication channel in use. These functions need to
 * be specified by the implementing class depending on the type of protocol adopted.
 */
class RemoteHandler {
public:
    /**
     * @brief Get a new instance of the remote message handler.
     *
     * @param [in] configuration The RemoteConfiguration to use.
     *
     * @returns A pointer to the remote message handler instantiated.
     */
    static std::unique_ptr<RemoteHandler> get_instance(const RemoteConfiguration &configuration);

    virtual ~RemoteHandler(){};

    // this class should not be copied or moved around
    RemoteHandler(const RemoteHandler &) = delete;
    RemoteHandler(RemoteHandler &&) = delete;

    /**
     * @brief Receive a new message by extracting the next available message from the MessageQueue.
     *
     * @param [out] input_message The message element receiving the data.
     *
     * @returns True if the reception was successful, False otherwise.
     */
    virtual bool recv_message(message_model &input_message) = 0;

    /**
     * @brief Send a new message on the communication channel.
     *
     * @param [in] output_message The message element storing the data to send.
     */
    virtual void send_message(const message_model &output_message) = 0;

    /**
     * @brief Subscribe to a new topic.
     *
     * @param [in] topic A string representing the topic to subscribe to.
     *
     * @details
     * This method needs to be implemented only if the specification is leveraging a Publish-Subscribe message protocol (e.g. Paho MQTT).
     */
    virtual void subscribe(const std::string &topic) = 0;

    /**
     * @brief Unsubscribe to an existing topic.
     *
     * @param [in] topic A string representing the topic to unsubscribe from.
     *
     * @details
     * This method needs to be implemented only if the specification is leveraging a Publish-Subscribe message protocol (e.g. Paho MQTT).
     */
    virtual void unsubscribe(const std::string &topic) = 0;

    /**
     * @brief Terminate the Agora broker connection to the communication channel.
     */
    virtual void disconnect() = 0;

    /**
     * @brief Get the unique identifier bound to the Agora broker.
     *
     * @returns A string representing the client ID.
     */
    virtual std::string get_my_client_id() const = 0;

protected:
    /**
     * @brief Constructor which uses the configuration provided to get a new instance based on the specified type.
     *
     * @param [in] configuration The RemoteConfiguration to use.
     */
    RemoteHandler(const RemoteConfiguration &configuration);
    /**
     * @brief The last configuration used by the factory method.
     */
    RemoteConfiguration configuration;

    /**
     * @brief A synchronized queue to store incoming messages.
     */
    MessageQueue inbox;
    /**
     * @brief A pointer to the global Logger.
     */
    std::shared_ptr<Logger> logger;

    /**
     * @brief Filter the specified message.
     *
     * @param [in, out] incoming_message The message to filter.
     *
     * @details
     * This method checks on an incoming message before adding it to the MessageQueue. It should be always called inside the recv_message()
     * function by the implementing class of this interface. Internally, it filters the message topic and payload accepting only the
     * following characters:
     *  - Topic: [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_/^.]
     *  - Payload: [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_ -.:,@<>=;()[]{}^*+'"]
     */
    void whitelist(message_model &incoming_message);
};

}  // namespace agora

#endif // REMOTE_HANDLER_HPP
