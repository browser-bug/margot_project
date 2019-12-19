#ifndef HEEL_PARSER_HDR
#define HEEL_PARSER_HDR

#include <stdexcept>
#include <vector>

#include <heel/configuration_file.hpp>
#include <heel/logger.hpp>
#include <heel/model_application.hpp>
#include <heel/parser_application.hpp>
#include <heel/parser_operating_point.hpp>

namespace margot {
namespace heel {

void parse(application_model& application, const configuration_file& conf_file) {
  // if the application does not contain any block, the configuration file describes the application
  if (application.blocks.empty()) {
    parse(application, conf_file.ptree());
  } else {
    // we have the application model. Therefore, the configuration file must contain the Operating Points of a
    // block. However, we don't know beforehand which block they relate, so we need to guess it
    for (auto& block_model : application.blocks) {
      parse_operating_points(block_model, conf_file.ptree());
      // double check that we don't have both, an Operating Point list and margot enabled on this block
      if (!block_model.agora.empty() && !block_model.ops.empty()) {
        margot::heel::error(
            "Both, the Operating Points list and Agora provide the application knowledge for block \"",
            block_model.name, "\", which one should i select?");
        throw std::runtime_error("workspace error: mismatch on application knowledge");
      }
    }
  }
}

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_HDR