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
#include <memory>


#include "agora/application_handler.hpp"


namespace agora
{

  class GlobalView
  {

    public:

      using RemoteApplicationHandlerPtr = std::shared_ptr<RemoteApplicationHandler>;

      static inline RemoteApplicationHandlerPtr get_handler( const std::string& application_name )
      {
        std::lock_guard<std::mutex> lock(global_structure);
        auto iterator = handled_applications.find(application_name);

        if (iterator == handled_applications.end())
        {
          auto&& application_ptr = RemoteApplicationHandlerPtr(new RemoteApplicationHandler(application_name));
          const auto result_pair = handled_applications.emplace(application_name, application_ptr);
          return result_pair.first->second;
        }

        return iterator->second;
      }

      static inline void remove_handler( const std::string& application_name )
      {
        std::lock_guard<std::mutex> lock(global_structure);
        auto iterator = handled_applications.find(application_name);

        if (iterator != handled_applications.end())
        {
          handled_applications.erase(iterator);
        }
      }

      // returns a string with the list of application_names which have active clients with the model.
      static inline std::vector<std::string> get_handlers_with_model()
      {
        std::lock_guard<std::mutex> lock(global_structure);

        std::vector<std::string> applications_with_model;

        for (auto iterator = handled_applications.begin(); iterator != handled_applications.end(); iterator++)
        {
          if ((iterator->second->get_status() == ApplicationStatus::WITH_MODEL))
          {
            applications_with_model.emplace_back(iterator->first);
          }
        }

        return applications_with_model;
      }

    private:

      static std::mutex global_structure;
      static std::unordered_map< std::string, RemoteApplicationHandlerPtr > handled_applications;
  };

}

#endif // MARGOT_AGORA_GLOBAL_VIEW_HDR
