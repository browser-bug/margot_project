#ifndef HEEL_CPP_INIT_GEN_HDR
#define HEEL_CPP_INIT_GEN_HDR

#include <algorithm>
#include <string>

#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/model_monitor.hpp>
#include <heel/model_parameter.hpp>

namespace margot {
namespace heel {

// this is an helper struct to write the declaration of input/output parameters
struct cpp_init_gen {
  static std::string signature(const application_model& app) {
    std::string signature_line;
    std::for_each(app.blocks.begin(), app.blocks.end(), [&signature_line](const block_model& block) {
      std::for_each(block.monitors.begin(), block.monitors.end(),
                    [&signature_line, &block](const monitor_model& monitor) {
                      std::for_each(
                          monitor.initialization_parameters.begin(), monitor.initialization_parameters.end(),
                          [&signature_line, &block](const parameter& p) {
                            if (p.type == parameter_types::VARIABLE) {
                              signature_line += !signature_line.empty() ? std::string(", ") : std::string("");
                              signature_line += p.value_type + " " + block.name + "_" + p.content;
                            }
                          });  // end for each initialization parameters
                    });        // end for each monitor
    });                        // end for each block
    return signature_line.empty() ? std::string("void") : signature_line;
  }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CPP_INIT_GEN_HDR