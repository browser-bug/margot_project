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

#include "common_objects.hpp"


namespace margot
{

  class FsHandler
  {
    public:

      virtual void store_metrics( const std::string& application_name, const application_metrics_t& metrics ) = 0;
      virtual application_metrics_t load_metrics( const std::string& application_name ) = 0;

      virtual void store_knobs( const std::string& application_name, const application_knobs_t& knobs ) = 0;
      virtual application_knobs_t load_knobs( const std::string& application_name ) = 0;

      virtual void store_features( const std::string& application_name, const application_features_t& features ) = 0;
      virtual application_features_t load_features( const std::string& application_name ) = 0;

      virtual void store_doe( const std::string& application_name, const doe_t& doe ) = 0;
      virtual doe_t load_doe( const std::string& application_name ) = 0;

      virtual void store_model( const std::string& application_name, const model_t& model ) = 0;
      virtual model_t load_model( const std::string& application_name ) = 0;


      virtual ~FsHandler( void ) {}
  };


}

#endif // MARGOT_AGORA_FS_HANDLER_HDR
