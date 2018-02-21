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
      inline void create_channel( const Ts& ... fs_arguments )
      {
        fs.reset( new T(fs_arguments...));
      }

      inline void store_metrics( const std::string& application_name, const application_metrics_t& metrics )
      {
        fs->store_metrics(application_name,metrics);
      }

      inline application_metrics_t load_metrics( const std::string& application_name )
      {
        return fs->load_metrics(application_name);
      }

      inline void store_knobs( const std::string& application_name, const application_knobs_t& knobs )
      {
        fs->store_knobs(application_name,knobs);
      }

      inline application_knobs_t load_knobs( const std::string& application_name )
      {
        return fs->load_knobs(application_name);
      }

      inline void store_features( const std::string& application_name, const application_features_t& features )
      {
        fs->store_features(application_name,features);
      }

      inline application_features_t load_features( const std::string& application_name )
      {
        return fs->load_features(application_name);
      }

      inline void store_model( const std::string& application_name, const model_t& model )
      {
        fs->store_model(application_name,model);
      }

      inline model_t load_model( const std::string& application_name )
      {
        return fs->load_model(application_name);
      }

      inline void store_doe( const std::string& application_name, const doe_t& doe )
      {
        fs->store_doe(application_name,doe);
      }

      inline doe_t load_doe( const std::string& application_name )
      {
        return fs->load_doe(application_name);
      }

  };

}

#endif // MARGOT_AGORA_VIRTUAL_FS_HDR
