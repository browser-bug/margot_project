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

enum class LoggerType { Console, File };

enum class LogLevel : uint8_t { DISABLED = 0, WARNING, INFO, PEDANTIC, DEBUG };
struct LoggerConfiguration
{
  LoggerConfiguration(LogLevel priority = LogLevel::DEBUG, LoggerType type = LoggerType::Console) : priority(priority), type(type) {}

  void set_file_logger_properties(const std::filesystem::path &log_file_path)
  {
    log_file = log_file_path;
  }

  LogLevel priority;
  LoggerType type;

  // file logger
  std::filesystem::path log_file;
};

} // namespace agora

#endif // LOGGER_CONFIGURATION_HPP
