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

#ifndef LOGGER_FILE_HPP
#define LOGGER_FILE_HPP

#include "agora/logger.hpp"

namespace agora {

/**
 * @brief Implementation of a Logger that saves messages on a log file.
 *
 * @note
 * Since logging messages to file is not a thread-safe action, the internal log function is mutex protected in order to enforce a
 * chronological order of events.
 */
class FileLogger : public Logger {
public:
    /**
     * @brief Construct a new instance opening a new log file to write.
     *
     * @param [in] configuration The LoggerConfiguration to use.
     */
    FileLogger(const LoggerConfiguration &configuration) : Logger(configuration) {
        log_file.open(configuration.log_file, std::ios::out | std::ios::trunc);
    };

    /**
     * @brief Destruct the instance closing the log file.
     */
    ~FileLogger() {
        if (log_file.is_open()) log_file.close();
    };

private:
    /**
     * @brief The output stream buffer which represents the log file.
     */
    std::ofstream log_file;
    /**
     * @brief The mutex used to enforce the chronological order of events.
     */
    std::mutex logger_mutex;

    /**
     * @brief Log to file a text message.
     *
     * @param [in] text A string representing the text message.
     *
     * @see Logger::log()
     */
    void log(const std::string &text) override {
        std::lock_guard<std::mutex> lock(logger_mutex);
        log_file << text << std::endl;
    }
};

}  // namespace agora

#endif // LOGGER_FILE_HPP
