/* mARGOt HEEL library
 * Copyright (C) 2018 Politecnico di Milano
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

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <heel/workspace.hpp>

int main(int argc, char* argv[]) {
  // define the program options to describe where the source files will be generated
  std::filesystem::path path_conf_file;
  std::filesystem::path path_workspace = std::filesystem::current_path();
  po::options_description desc("Allowed options");
  // clang-format off
  desc.add_options()
     ("help,h", "prints this message")
     ("configuration_file,c", po::value<std::filesystem::path>(&path_conf_file)->required(),
      "mARGOt configuration file path")
     ("workspace,w", po::value<std::filesystem::path>(&path_workspace)->default_value(path_workspace),
      "output folder path (cwd as default)")
     ("op_files,o", po::value<std::vector<std::filesystem::path>>(),
      "operating points file path")
  ;
  // clang-format on
  po::positional_options_description p;
  p.add("op_files", -1);

  // parse the program options
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  if (vm.count("help") > 0) {
    std::cout << "This application generates the high-level interface of the mARGOt" << std::endl;
    std::cout << "dynamic autotuning framework. Moreover, it prints in the standard" << std::endl;
    std::cout << "output the list of generated files." << std::endl << std::endl;
    std::cout << "Usage: " << argv[0] << " [options] [path_op_list]*" << std::endl;
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }
  po::notify(vm);
  auto op_list_files = !vm["op_files"].empty() ? vm["op_files"].as<std::vector<std::filesystem::path>>()
                                               : std::vector<std::filesystem::path>();

  // now we need to post-process the configuration paths to make sure to work with absolute paths. So, if we
  // have relative paths, we need to rebase them with respect to the current working directory
  const auto rebase_path = [](std::filesystem::path& p) {
    p = p.has_root_path() ? p : std::filesystem::current_path() / p;
  };
  rebase_path(path_workspace);
  rebase_path(path_conf_file);
  std::for_each(op_list_files.begin(), op_list_files.end(),
                [&rebase_path](std::filesystem::path& p) { rebase_path(p); });

  // create and initialize the workspace
  margot::heel::workspace driver(path_workspace, path_conf_file, op_list_files);

  // generate the high-level adaptive interface for the application
  driver.generate_adaptive_interface();
  return EXIT_SUCCESS;
}