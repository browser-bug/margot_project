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

#ifndef LOGGER_CONSOLE_HPP
#define LOGGER_CONSOLE_HPP

#include "agora/logger.hpp"

namespace agora {

class ConsoleLogger : public Logger {

public:
  ConsoleLogger(const LoggerConfiguration &configuration) : Logger(configuration){};

private:
  // std::cout is thread safe but we need to aboid multiple threads message interleaving on one line.
  // So only call the stream output operator << once.
  void log(const std::string &text) override
  {
    std::cout << text + "\n";
    std::cout.flush();
  }
};

} // namespace agora

#endif // LOGGER_CONSOLE_HPP
