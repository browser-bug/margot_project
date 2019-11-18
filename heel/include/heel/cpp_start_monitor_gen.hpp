#ifndef HEEL_CPP_START_MONITOR_GEN_HDR
#define HEEL_CPP_START_MONITOR_GEN_HDR

#include <algorithm>
#include <string>

#include <heel/generator_utils.hpp>
#include <heel/model_block.hpp>
#include <heel/model_monitor.hpp>
#include <heel/model_parameter.hpp>

namespace margot {
namespace heel {

// this is an helper struct to write the declaration of input/output parameters
struct cpp_start_monitor_gen {
  static std::string signature(const block_model& block) {
    std::string signature_line;
    std::for_each(block.monitors.begin(), block.monitors.end(), [&signature_line](const monitor_model& m) {
      std::for_each(m.start_parameters.begin(), m.start_parameters.end(),
                    [&signature_line](const parameter& p) {
                      if (p.type == parameter_types::VARIABLE) {
                        signature_line += !signature_line.empty() ? std::string(", ") : std::string("");
                        signature_line += "const " + p.value_type + " " + p.content;
                      }
                    });
    });
    return signature_line.empty() ? std::string("void") : signature_line;
  }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CPP_START_MONITOR_GEN_HDR