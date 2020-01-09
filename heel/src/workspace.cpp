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
#include <stdexcept>
#include <vector>

#include <heel/composer.hpp>
#include <heel/configuration_file.hpp>
#include <heel/generator_cpp_application_geometry_hdr.hpp>
#include <heel/generator_cpp_application_geometry_src.hpp>
#include <heel/generator_cpp_knowledge_hdr.hpp>
#include <heel/generator_cpp_knowledge_src.hpp>
#include <heel/generator_cpp_managers_hdr.hpp>
#include <heel/generator_cpp_managers_src.hpp>
#include <heel/generator_cpp_margot_hdr.hpp>
#include <heel/generator_cpp_margot_src.hpp>
#include <heel/generator_source_file.hpp>
#include <heel/logger.hpp>
#include <heel/model_application.hpp>
#include <heel/model_validate.hpp>
#include <heel/parser.hpp>
#include <heel/workspace.hpp>

namespace margot {
namespace heel {

workspace::workspace(const std::filesystem::path& root_path, const std::filesystem::path& margot_config_path,
                     const std::vector<std::filesystem::path>& ops_config_path)
    : project_root(root_path), path_configuration_files(ops_config_path) {
  // we start by parsing and validating the application model
  configuration_file c;
  c.load_json(margot_config_path);
  path_configuration_files.emplace_back(margot_config_path);
  parse(model, c);
  validate(model);

  // then, we need to parse all the operting points file(s), if any
  std::for_each(ops_config_path.begin(), ops_config_path.end(), [this](const std::filesystem::path& p) {
    configuration_file op_config_file;
    op_config_file.load_json(p);
    parse(model, op_config_file);
  });
}

void workspace::generate_adaptive_interface(void) {
  // define the root path for the source files that compose the application
  const std::filesystem::path hdr_path = project_root / "include";
  const std::filesystem::path src_path = project_root / "src";

  // generate the content of the header files of the high level interface
  std::vector<source_file_generator> headers = {
      {hdr_path / "margot" / "application_geometry.hpp", application_geometry_hpp_content(model)},
      {hdr_path / "margot" / "managers_definition.hpp", managers_hpp_content(model)},
      {hdr_path / "margot" / "application_knowledge.hpp", knowledge_hpp_content(model)},
      {hdr_path / "margot" / "margot.hpp", margot_hpp_content(model)}};

  // generate the description of the model, after the validation
  configuration_file c;
  compose(c, model);

  // generate the content of the source files of the high level interface
  std::vector<source_file_generator> sources = {
      {src_path / "application_geometry.cpp", application_geometry_cpp_content(model)},
      {src_path / "managers_definition.cpp", managers_cpp_content(model, c.to_json_string())},
      {src_path / "application_knowledge.cpp", knowledge_cpp_content(model)},
      {src_path / "margot.cpp", margot_cpp_content(model)}};

  // everything has been parsed and validated. The content of the interface has been generated. The only thing
  // left is to actually write the interface on the given path
  std::filesystem::create_directories(hdr_path / "margot");
  std::filesystem::create_directories(src_path);
  std::for_each(headers.begin(), headers.end(),
                [this](source_file_generator& g) { g.write_header(path_configuration_files); });
  std::for_each(sources.begin(), sources.end(),
                [this](source_file_generator& g) { g.write_source(path_configuration_files); });
}

}  // namespace heel
}  // namespace margot