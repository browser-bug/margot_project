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

margot::heel::workspace::workspace(const std::filesystem::path& root_path,
                                   const std::filesystem::path& margot_config_path,
                                   const std::vector<std::filesystem::path>& ops_config_path)
    : project_root(root_path), path_configuration_files(ops_config_path) {
  // we start by parsing and validating the application model
  margot::heel::configuration_file c;
  c.load_json(margot_config_path);
  model = margot::heel::parse(c);
  margot::heel::validate(model);

  // now we need to check, for each block, if we have operating points. If so, agora must be disabled
  std::for_each(model.blocks.begin(), model.blocks.end(), [&ops_config_path](block_model& block) {
    // at this point we append all the points from the Operating Points list
    std::for_each(ops_config_path.begin(), ops_config_path.end(), [&block](const std::filesystem::path& p) {
      margot::heel::configuration_file op_config_file;
      op_config_file.load_json(p);
      auto new_ops(margot::heel::parse(op_config_file, block));
      block.ops.insert(block.ops.end(), new_ops.begin(), new_ops.end());
    });

    // double check that we don't have both, an Operating Point list and margot enabled on this block
    if (block.agora.enabled && !block.ops.empty()) {
      margot::heel::error(
          "Both, the Operating Points list and Agora provide the application knowledge, which one should i "
          "select?");
      throw std::runtime_error("workspace error: mismatch on application knowledge");
    }
  });

  // finally, append the margot configuration file to vector of configuration files
  path_configuration_files.emplace_back(margot_config_path);
}

void margot::heel::workspace::generate_adaptive_interface(void) {
  // define the root path for the source files that compose the application
  const std::filesystem::path hdr_path = project_root / "include";
  const std::filesystem::path src_path = project_root / "src";

  // generate the content of the header files of the high level interface
  std::vector<margot::heel::source_file_generator> headers = {
      {hdr_path / "margot" / "application_geometry.hpp",
       margot::heel::application_geometry_hpp_content(model)},
      {hdr_path / "margot" / "managers_definition.hpp", margot::heel::managers_hpp_content(model)},
      {hdr_path / "margot" / "application_knowledge.hpp", margot::heel::knowledge_hpp_content(model)},
      {hdr_path / "margot" / "margot.hpp", margot::heel::margot_hpp_content(model)}};

  // generate the description of the model, after the validation
  margot::heel::configuration_file c;
  margot::heel::compose(c, model);

  // generate the content of the source files of the high level interface
  std::vector<margot::heel::source_file_generator> sources = {
      {src_path / "application_geometry.cpp", margot::heel::application_geometry_cpp_content(model)},
      {src_path / "managers_definition.cpp", margot::heel::managers_cpp_content(model, c.to_json_string())},
      {src_path / "application_knowledge.cpp", margot::heel::knowledge_cpp_content(model)},
      {src_path / "margot.cpp", margot::heel::margot_cpp_content(model)}};

  // everything has been parsed and validated. The content of the interface has been generated. The only thing
  // left is to actually write the interface on the given path
  std::filesystem::create_directories(hdr_path / "margot");
  std::filesystem::create_directories(src_path);
  std::for_each(headers.begin(), headers.end(),
                [this](margot::heel::source_file_generator& g) { g.write_header(path_configuration_files); });
  std::for_each(sources.begin(), sources.end(),
                [this](margot::heel::source_file_generator& g) { g.write_source(path_configuration_files); });
}