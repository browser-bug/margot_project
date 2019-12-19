#ifndef HEEL_GENERATOR_CPP_MARGOT_SRC_HDR
#define HEEL_GENERATOR_CPP_MARGOT_SRC_HDR

#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>

namespace margot {
namespace heel {

// this function generates the actual high-level interface
cpp_source_content margot_cpp_content(const application_model& app);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_CPP_MARGOT_SRC_HDR