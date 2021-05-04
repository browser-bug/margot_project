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

#include <cstdarg>

#include "agora/logger.hpp"
#include "agora/logger_console.hpp"
#include "agora/logger_file.hpp"

using namespace agora;

std::unique_ptr<Logger> Logger::get_instance(const LoggerConfiguration &configuration)
{
  std::unique_ptr<Logger> logger;

  switch (configuration.type)
  {
  case LoggerType::Console:
    logger = std::make_unique<ConsoleLogger>(configuration);
    break;
  case LoggerType::File:
    logger = std::make_unique<FileLogger>(configuration);
    break;
  default:
    logger = std::make_unique<ConsoleLogger>(configuration);
    logger->warning("Unknown logger type. Setting default console logger.");
  }

  return logger;
}

const std::string Logger::get_time_as_string() const
{
  const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::ostringstream oss;
  oss << std::put_time(std::localtime(&now), "%F %T");
  return oss.str();
}
