/* beholder/global_view_beholder.hpp
 * Copyright (C) 2018 Alberto Bendin
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

#ifndef MARGOT_BEHOLDER_GLOBAL_VIEW_HDR
#define MARGOT_BEHOLDER_GLOBAL_VIEW_HDR


#include <mutex>
#include <unordered_map>
#include <string>
#include <memory>


#include "beholder/application_handler_beholder.hpp"


namespace beholder
{

  class GlobalView
  {
      static bool with_agora;

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

      // TODO: check if this method can be deleted
      static inline void remove_all_handlers()
      {
        std::lock_guard<std::mutex> lock(global_structure);

        for (auto iterator = handled_applications.begin(); iterator != handled_applications.end(); iterator++)
        {
          handled_applications.erase(iterator);
        }
      }

      static inline bool is_managing( const std::string& application_name )
      {
        std::lock_guard<std::mutex> lock(global_structure);
        auto iterator = handled_applications.find(application_name);

        if (iterator != handled_applications.end())
        {
          return true;
        }

        return false;
      }

      static inline bool is_with_agora()
      {
        std::lock_guard<std::mutex> lock(global_structure);
        return with_agora;
      }

      static inline void set_with_agora_true()
      {
        std::lock_guard<std::mutex> lock(global_structure);
        with_agora = true;
      }

      static inline void set_with_agora_false()
      {
        std::lock_guard<std::mutex> lock(global_structure);
        with_agora = false;
      }

      static inline void set_handlers_disabled()
      {
        std::lock_guard<std::mutex> lock(global_structure);

        for (auto iterator = handled_applications.begin(); iterator != handled_applications.end(); iterator++)
        {
          iterator->second->pause_handler();
        }
      }

      static inline void set_handlers_enabled()
      {
        std::lock_guard<std::mutex> lock(global_structure);

        for (auto iterator = handled_applications.begin(); iterator != handled_applications.end(); iterator++)
        {
          iterator->second->un_pause_handler();
        }
      }

      static inline int get_handlers_number()
      {
        std::lock_guard<std::mutex> lock(global_structure);

        return handled_applications.size();
      }


    private:

      static std::mutex global_structure;
      static std::unordered_map< std::string, RemoteApplicationHandlerPtr > handled_applications;
  };

}

#endif // MARGOT_BEHOLDER_GLOBAL_VIEW_HDR
