/* agora/application_handler.cc
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


#include "application_handler.hpp"
#include "logger.hpp"


using namespace margot;

RemoteApplicationHandler::RemoteApplicationHandler( const std::string& application_name )
  : application_name(application_name), status(ApplicationStatus::REQUESTING_INFO)
{}


void RemoteApplicationHandler::welcome_application( const std::string& application_name )
{
  switch (status)
  {
    case ApplicationStatus::REQUESTING_INFO:
      break;

    default:
      warning("Remote handler: the application \"", application_name, "\" is arrived in an unknown status: ", static_cast<uint_fast8_t>(status));
  }
}
