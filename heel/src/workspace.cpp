#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <vector>

#include <heel/configuration_file.hpp>
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

  // then, we load all the configuration files (if agora is not enabled)
  std::for_each(ops_config.begin(), ops_config.end(), [this](const std::filesystem::path& p) {
    margot::heel::configuration_file op_config_file;
    op_config_file.load(p);
    margot::heel::parse_json(op_config_file, model);
  });
}

void margot::heel::workspace::generate_adaptive_interface(void) {}