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

#ifndef LOGGER_CONFIGURATION_HPP
#define LOGGER_CONFIGURATION_HPP

#include <filesystem>
#include <string>

namespace agora {

/**
 * @brief Available Logger implementations.
 *
 * @details
 * These values represents a list of available logger implementations which specifies a Logger interface.
 */
enum class LoggerType {
    Console,  ///< A logger which uses the standard output channel to show messages on the console/terminal.
    File      ///< A logger which writes the output on a file, appending messages in order.
};

/**
 * @brief Available logging levels.
 *
 * @details
 * These values represents the types of events to register and which to ignore instead.
 */
enum class LogLevel : uint8_t {
    DISABLED = 0,  ///< Disable any type of logs.
    WARNING,       ///< Log an abnormal or unexpected event in the application flow.
    INFO,          ///< Log the general flow of the application.
    PEDANTIC,      ///< Log the most detailed messages which may contain sensitive application data.
    DEBUG          ///< Log used for interactive investigation during development.
};

/**
 * @brief A generic configuration for a Logger.
 *
 * @details
 * This data structure contains the specification for a generic logger. This includes the level of priority of the messages and the type of
 * logger to use.
 */
struct LoggerConfiguration {
    /**
     * @brief Construct a new Logger configuration.
     *
     * @param [in] priority The message priority to use during the event recording.
     * @param [in] type The type of logger which implements the logging function.
     */
    LoggerConfiguration(LogLevel priority = LogLevel::DEBUG, LoggerType type = LoggerType::Console) : priority(priority), type(type) {}

    /**
     * @brief Set the log location where the messages will be stored.
     *
     * @param [in] log_file_path The filesystem path to the log file.
     */
    void set_file_logger_properties(const std::filesystem::path &log_file_path) { log_file = log_file_path; }

    /// The logging level to use.
    LogLevel priority;
    /// The logger type implementation to use.
    LoggerType type;

    /// The filesystem path to the log file.
    std::filesystem::path log_file;
};

}  // namespace agora

#endif // LOGGER_CONFIGURATION_HPP
