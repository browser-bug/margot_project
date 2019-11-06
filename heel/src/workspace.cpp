#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <vector>

#include <heel/configuration_file.hpp>
#include <heel/generator_cpp_application_geometry_hdr.hpp>
#include <heel/generator_cpp_application_geometry_src.hpp>
#include <heel/generator_source_file.hpp>
#include <heel/json_parser.hpp>
#include <heel/logger.hpp>
#include <heel/model_application.hpp>
#include <heel/model_validate.hpp>
#include <heel/workspace.hpp>

margot::heel::workspace::workspace(const std::filesystem::path& root_path,
                                   const std::filesystem::path& margot_config_path,
                                   const std::vector<std::filesystem::path>& ops_config_path)
    : project_root(root_path), application_config(margot_config_path), ops_config(ops_config_path) {
  // we start by parsing and validating the application model
  margot::heel::configuration_file c;
  c.load(margot_config_path);
  model = margot::heel::parse_json(c);
  margot::heel::validate(model);

  // now we need to check, for each block, if we have operating points. If so, agora must be disabled
  std::for_each(model.blocks.begin(), model.blocks.end(), [this](block_model& block) {
    // at this point we append all the points from the Operating Points list
    std::for_each(ops_config.begin(), ops_config.end(), [&block](const std::filesystem::path& p) {
      margot::heel::configuration_file op_config_file;
      op_config_file.load(p);
      auto new_ops(margot::heel::parse_json(op_config_file, block));
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
}

void margot::heel::workspace::generate_adaptive_interface(void) {
  // define all the source files that composes the high-level interface
  const std::filesystem::path hdr_path = project_root / "include";
  const std::filesystem::path src_path = project_root / "src";
  const std::filesystem::path application_geometry_hdr_path = hdr_path / "application_geometry.hpp";
  const std::filesystem::path application_geometry_src_path = src_path / "application_geometry.cpp";

  // generate the high-level interface content. This operation is separated from the actual file generation to
  // avoid the generation of an incomplete interface
  margot::heel::source_file_generator application_geometry_hdr(
      application_geometry_hdr_path, margot::heel::application_geometry_hpp_content(model));
  margot::heel::source_file_generator application_geometry_src(
      application_geometry_src_path, margot::heel::application_geometry_cpp_content(model));

  // everything has been parsed and validated. The content of the interface has been generated. The only thing
  // left to actually generates the interface
  std::filesystem::create_directories(hdr_path);
  std::filesystem::create_directories(src_path);
  application_geometry_hdr.write_header(application_config);
  application_geometry_src.write_source(application_config);
}