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

class FileLogger : public Logger {
public:
    FileLogger(const LoggerConfiguration &configuration) : Logger(configuration) {
        log_file.open(configuration.log_file, std::ios::out | std::ios::trunc);
    };

    ~FileLogger() {
        if (log_file.is_open()) log_file.close();
    };

private:
    std::ofstream log_file;
    std::mutex logger_mutex;

    void log(const std::string &text) override {
        std::lock_guard<std::mutex> lock(logger_mutex);
        log_file << text << std::endl;
    }
};

}  // namespace agora

#endif  // LOGGER_FILE_HPP
