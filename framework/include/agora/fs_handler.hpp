/* agora/fs_handler.hpp
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

#ifndef MARGOT_AGORA_FS_HANDLER_HDR
#define MARGOT_AGORA_FS_HANDLER_HDR


#include <string>
#include <vector>

#include "agora/common_objects.hpp"


namespace agora
{

  class FsHandler
  {
    public:

      virtual void store_description( const application_description_t& description ) = 0;
      virtual application_description_t load_description( const std::string& application_name ) = 0;

      virtual void store_doe( const application_description_t& description, const doe_t& doe ) = 0;
      virtual doe_t load_doe( const std::string& application_name ) = 0;
      virtual void update_doe( const application_description_t& description, const std::string& values ) = 0;
      virtual void reset_doe( const application_description_t& description ) = 0;

      virtual void store_model( const application_description_t& description, const model_t& model ) = 0;
      virtual model_t load_model( const std::string& application_name ) = 0;

      virtual void create_trace_table( const application_description_t& description ) = 0;
      virtual void insert_trace_entry( const application_description_t& description, const std::string& values ) = 0;

      virtual application_list_t load_clients( const std::string& application_name ) = 0;

      virtual void erase( const std::string& application_name ) = 0;

      virtual std::string get_type( void ) const = 0;
      virtual std::string get_address( void ) const = 0;
      virtual std::string get_username( void ) const = 0;
      virtual std::string get_password( void ) const = 0;
      virtual std::string get_observation_name( const std::string& application_name ) const = 0;
      virtual std::string get_model_name( const std::string& application_name ) const = 0;
      virtual std::string get_knobs_name( const std::string& application_name ) const = 0;
      virtual std::string get_features_name( const std::string& application_name ) const = 0;


      virtual ~FsHandler( void ) {}
  };


}

#endif // MARGOT_AGORA_FS_HANDLER_HDR
