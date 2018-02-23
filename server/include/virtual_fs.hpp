/* agora/virtual_fs.hpp
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


#ifndef MARGOT_AGORA_VIRTUAL_FS_HDR
#define MARGOT_AGORA_VIRTUAL_FS_HDR

#include <memory>
#include <cassert>

#include "fs_handler.hpp"
#include "cassandra_fs_implementation.hpp"

namespace margot
{

  class VirtualFs
  {
    private:

      std::shared_ptr< FsHandler > fs;

    public:

      template< class T, class ...Ts >
      inline void create( const Ts& ... fs_arguments )
      {
        fs.reset( new T(fs_arguments...));
      }

      inline void store_description( const application_description_t& description )
      {
        fs->store_description(description);
      }

      inline application_description_t load_description( const std::string& application_name )
      {
        return fs->load_description(application_name);
      }

      inline void store_model( const application_description_t& description, const model_t& model )
      {
        fs->store_model(description, model);
      }

      inline model_t load_model( const std::string& application_name )
      {
        return fs->load_model(application_name);
      }

      inline void store_doe( const application_description_t& description, const doe_t& doe )
      {
        fs->store_doe(description, doe);
      }

      inline doe_t load_doe( const std::string& application_name )
      {
        return fs->load_doe(application_name);
      }

      inline void update_doe( const application_description_t& description, const std::string& values )
      {
        return fs->update_doe(description, values);
      }

      inline void create_trace_table( const application_description_t& description )
      {
        fs->create_trace_table(description);
      }

      inline void insert_trace_entry( const application_description_t& description, const std::string& values )
      {
        fs->insert_trace_entry(description, values);
      }

      inline std::string get_type( void ) const
      {
        return fs->get_type();
      }
      inline std::string get_address( void ) const
      {
        return fs->get_address();
      }
      inline std::string get_username( void ) const
      {
        return fs->get_username();
      }
      inline std::string get_password( void ) const
      {
        return fs->get_password();
      }
      inline std::string get_observation_name( const std::string& application_name ) const
      {
        return fs->get_observation_name(application_name);
      }
      inline std::string get_model_name( const std::string& application_name ) const
      {
        return fs->get_model_name(application_name);
      }
      inline std::string get_knobs_name( const std::string& application_name ) const
      {
        return fs->get_knobs_name(application_name);
      }
      inline std::string get_features_name( const std::string& application_name ) const
      {
        return fs->get_features_name(application_name);
      }

  };

}

#endif // MARGOT_AGORA_VIRTUAL_FS_HDR
