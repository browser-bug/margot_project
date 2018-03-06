/* agora/logger.cc
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

#include "agora/logger.hpp"



using namespace agora;

Logger agora::my_agora_logger;

Logger::Logger( void )
  : filter_level(static_cast<uint8_t>(LogLevel::DEBUG))
{
  //log_file.open("margot_agora.log", std::ofstream::out | std::ofstream::app);
}

Logger::~Logger( void )
{
  //if (log_file.is_open())
  //{
  //  log_file.close();
  //}
}


void Logger::set_filter_at(const LogLevel new_minimum_log_level)
{
  filter_level = static_cast<uint8_t>(new_minimum_log_level);
}
