/* agora/global_view.hpp
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

#ifndef MARGOT_AGORA_GLOBAL_VIEW_HDR
#define MARGOT_AGORA_GLOBAL_VIEW_HDR


#include <mutex>
#include <unordered_map>
#include <string>


#include "application_handler.hpp"
#include "virtual_channel.hpp"


namespace margot
{

  class GlobalView
  {

    private:

      std::mutex global_structure;
      std::unordered_map< std::string, RemoteApplicationHandler > handled_applications;

    public:






  };

}

#endif // MARGOT_AGORA_GLOBAL_VIEW_HDR
