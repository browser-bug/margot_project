/* mARGOt HEEL library
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

#ifndef HEEL_WORKSPACE_HDR
#define HEEL_WORKSPACE_HDR

#include <filesystem>
#include <string>
#include <vector>

#include <heel/model_application.hpp>

namespace margot {
namespace heel {

// this is the main class that drives the generation of the high level interface, including the required CMake
// files and finders. The idea is to generate on a specific path everything required to perform an import of
// the target as an internal cmake project, to minimize the integration effort.
class workspace {
  // this represents the path to the root output directory
  std::filesystem::path project_root;

  // this is the whole model of the application that we need to manage
  application_model model;

  // these are the path of the configuration files used to create the application model. We need to keep track
  // of them to write in the source file the path of the configuration file that has generated the interface,
  // for clarity reasons
  std::vector<std::filesystem::path> path_configuration_files;

 public:
  // in the workspace we perform all the parsing of the configuration files and the model validation
  workspace(const std::filesystem::path& root_path, const std::filesystem::path& margot_config_path,
            const std::vector<std::filesystem::path>& ops_config_path);

  // this function actually generates the high level interface, writing on the file system
  void generate_adaptive_interface(void);
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_WORKSPACE_HDR