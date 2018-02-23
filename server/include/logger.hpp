/* agora/logger.hpp
 * Copyright (C) 2018 Davide Gadioli
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

#ifndef MARGOT_AGORA_LOGGER_HDR
#define MARGOT_AGORA_LOGGER_HDR

#include <fstream>
#include <string>
#include <cstdint>
#include <chrono>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <mutex>


namespace margot
{


  enum class LogLevel : uint8_t
  {
    WARNING = 0,
    INFO,
    PEDANTIC,
    DEBUG
  };

  inline const std::string get_string_level( const LogLevel level )
  {
    switch (static_cast<uint8_t>(level))
    {
      case static_cast<uint8_t>(LogLevel::WARNING):
        return "Warning ";

      case static_cast<uint8_t>(LogLevel::INFO):
        return "Info    ";

      case static_cast<uint8_t>(LogLevel::PEDANTIC):
        return "Pedantic";

      case static_cast<uint8_t>(LogLevel::DEBUG):
        return "Debug   ";

      default:
        return "Undef   ";
    }
  }



  class Logger
  {
    private:

      //std::ofstream log_file;
      uint8_t filter_level;
      std::mutex sync_mutex;


    public:

      Logger( void );

      ~Logger( void );

      void set_filter_at( const LogLevel new_minimum_log_level );

      template<class ...T>
      inline void log( const LogLevel level, const T... payload)
      {
        if (static_cast<uint8_t>(level) <= filter_level)
        {
          std::lock_guard<std::mutex> lock(sync_mutex);
          const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
          internal_log(std::put_time(std::localtime(&now), "%F %T"), " [", get_string_level(level), "] ", payload...);
        }
      }

    protected:

      template< class T, class ...Ts >
      inline void internal_log( const T first_argument, const Ts... remainder )
      {
        std::cout << first_argument;
        internal_log(remainder...);
      }

      template< class T >
      inline void internal_log( const T last_argument )
      {
        std::cout << last_argument << std::endl;
      }

  };

  extern Logger my_agora_logger;


  template< class ...T>
  inline void warning(const T... arguments)
  {
    my_agora_logger.log(LogLevel::WARNING, arguments...);
  }

  template< class ...T>
  inline void info(const T... arguments)
  {
    my_agora_logger.log(LogLevel::INFO, arguments...);
  }

  template< class ...T>
  inline void pedantic(const T... arguments)
  {
    my_agora_logger.log(LogLevel::PEDANTIC, arguments...);
  }

  template< class ...T>
  inline void debug(const T... arguments)
  {
    my_agora_logger.log(LogLevel::DEBUG, arguments...);
  }


}

#endif // MARGOT_AGORA_LOGGER_HDR
