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

#ifndef MARGOT_AGORA_LAUNCHER_HDR
#define MARGOT_AGORA_LAUNCHER_HDR

#include <string>

#include "agora/common_objects.hpp"

namespace agora {

// This class enumerates all the available lanchers
enum class LauncherType { ModelGenerator, DoeGenerator };

// This is a generic class that implement a launcher for a plugin
// the actual behavior depends on its type
template <LauncherType type>
class Launcher {
 private:
  // this is the path of the root workspace used to
  // generate an application model
  std::string workspace_root;

  // this is the path of the folder that contains all
  // the available plugin to compute the model
  std::string plugins_folder;

  // this is the name of the environmental file that
  // describes the parameters of the agora execution
  const std::string config_file_name;

  // this is the name of the bash script that launch
  // the actual plugin folder
  const std::string script_file_name;

 public:
  Launcher(void) : config_file_name("agora_config.env"), script_file_name("generate_model.sh") {}

  inline void initialize(const std::string& workspace_path, const std::string& plugins_path) {
    workspace_root = workspace_path;
    plugins_folder = plugins_path;

    if (workspace_root.back() != '/') {
      workspace_root.append("/");
    }

    if (plugins_folder.back() != '/') {
      plugins_folder.append("/");
    }
  }

  // the application description is an input parameter, the model is an input/output parameter
  void operator()(const application_description_t& application, const uint_fast32_t iteration_counter) const;
};

}  // namespace agora

#endif  // MARGOT_AGORA_LAUNCHER_HDR
