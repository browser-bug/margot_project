/* agora/application_handler.hpp
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


#ifndef MARGOT_AGORA_APPLICATION_HANDLER_HDR
#define MARGOT_AGORA_APPLICATION_HANDLER_HDR

#include <string>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>

#include "virtual_channel.hpp"
#include "common_objects.hpp"

namespace margot
{

  enum class ApplicationStatus : uint_fast8_t
  {
    REQUESTING_INFO,
    DOE_GENERATION,
    DESIGN_SPACE_EXPLORATION,
    MODEL_GENERATION,
    MANAGED
  };


  class RemoteApplicationHandler
  {


    private:

      std::mutex handler_mutex;
      ApplicationStatus status;
      VirtualChannel channel;

      application_list_t pending_application;
      application_map_t assigned_configurations;

    public:


      RemoteApplicationHandler( VirtualChannel channel );


      void welcome_application( const std::string& application_name );



  };

}

#endif // MARGOT_AGORA_APPLICATION_HANDLER_HDR
