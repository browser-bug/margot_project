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

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <chrono>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <utility>

#include "agora/agora_properties.hpp"
#include "agora/logger_configuration.hpp"

namespace agora {

/**
 * @brief Interface representing a generic logger entity.
 *
 * @details
 * This interface implements a factory pattern which enables the user to get a new instance of the class deriving from it depending on the
 * provided configuration. Provide a public API containing the methods corresponding to the log level which internally uses the `log`
 * function, specified by the implementing class.
 */
class Logger {
public:
    /**
     * @brief Get a new instance of the logger.
     *
     * @param [in] configuration The LoggerConfiguration to use.
     *
     * @returns A pointer to the logger instantiated.
     */
    static std::unique_ptr<Logger> get_instance(const LoggerConfiguration &configuration);

    virtual ~Logger(){};

    // this class should not be copied or moved around
    Logger(Logger const &) = delete;
    void operator=(Logger const &) = delete;

    /**
     * @brief Log a message with DEBUG priority.
     *
     * @tparam T The message data type to log.
     * @param [in] args A list of message elements to log.
     */
    template <typename... T>
    void debug(const T &...args) {
        if (static_cast<uint8_t>(LogLevel::DEBUG) <= static_cast<uint8_t>(configuration.priority)) {
            std::string final_text = get_time_as_string() + " [DEBUG] " + get_log_text(args...);
            log(final_text);
        }
    }
    /**
     * @brief Log a message with INFO priority.
     *
     * @tparam T The message data type to log.
     * @param [in] args A list of message elements to log.
     */
    template <typename... T>
    void info(const T &...args) {
        if (static_cast<uint8_t>(LogLevel::INFO) <= static_cast<uint8_t>(configuration.priority)) {
            std::string final_text = get_time_as_string() + " [INFO] " + get_log_text(args...);
            log(final_text);
        }
    }
    /**
     * @brief Log a message with WARNING priority.
     *
     * @tparam T The message data type to log.
     * @param [in] args A list of message elements to log.
     */
    template <typename... T>
    void warning(const T &...args) {
        if (static_cast<uint8_t>(LogLevel::WARNING) <= static_cast<uint8_t>(configuration.priority)) {
            std::string final_text = get_time_as_string() + " [WARNING] " + get_log_text(args...);
            log(final_text);
        }
    }
    /**
     * @brief Log a message with PEDANTIC priority.
     *
     * @tparam T The message data type to log.
     * @param [in] args A list of message elements to log.
     */
    template <typename... T>
    void pedantic(const T &...args) {
        if (static_cast<uint8_t>(LogLevel::PEDANTIC) <= static_cast<uint8_t>(configuration.priority)) {
            std::string final_text = get_time_as_string() + " [PEDANTIC] " + get_log_text(args...);
            log(final_text);
        }
    }

    /**
     * @brief Set a new logging priority level.
     *
     * @param [in] new_log_level The new level to apply.
     */
    void set_log_filter(const LogLevel &new_log_level) { configuration.priority = new_log_level; }

protected:
    /**
     * @brief Constructor which uses the configuration provided to get a new instance based on the specified type.
     *
     * @param [in] configuration The LoggerConfiguration to use.
     */
    Logger(const LoggerConfiguration &configuration) : configuration(configuration){};

    /**
     * @brief Convert a message to string format.
     *
     * @tparam T The message data type to convert.
     * @param [in] args A list of message elements to convert.
     */
    template <typename... T>
    const std::string get_log_text(T &...args) const {
        std::ostringstream log_text;
        ((log_text << std::forward<T>(args)), ...);
        return log_text.str();
    }

    /**
     * @brief Convert the current local time to string format.
     *
     * @returns A string representing the current local time.
     */
    const std::string get_time_as_string() const;

    /**
     * @brief Log the message text.
     *
     * @param [in] text A string representing the message text.
     *
     * @details
     * This is the internal log function that needs to be specified by every type of Logger.
     */
    virtual void log(const std::string &text) = 0;

    /**
     * @brief The last configuration used by the factory method.
     */
    LoggerConfiguration configuration;
};

}  // namespace agora

#endif // LOGGER_HPP
