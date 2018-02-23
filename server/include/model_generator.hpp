/* agora/model_generator.hpp
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

#ifndef MARGOT_AGORA_MODEL_GENERATOR_HDR
#define MARGOT_AGORA_MODEL_GENERATOR_HDR

#include <string>

#include "common_objects.hpp"

namespace margot
{

  class ModelGenerator
  {

    private:

      // this is the path of the root workspace used to
      // generate an application model
      std::string workspace_root;

      // this is the path of the folder that contains all
      // the available plugin to compute the model
      std::string plugins_folder;


    public:

      inline void initialize( const std::string& workspace_path, const std::string& plugins_path)
      {
        workspace_root = workspace_path;
        plugins_folder = plugins_path;
      }

      // the application description is an input parameter, the model is an input/output parameter
      void operator()( const application_description_t& application ) const;


  };

}

#endif // MARGOT_AGORA_MODEL_GENERATOR_HDR
