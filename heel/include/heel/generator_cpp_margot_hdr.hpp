#ifndef HEEL_GENERATOR_CPP_MARGOT_HDR_HDR
#define HEEL_GENERATOR_CPP_MARGOT_HDR_HDR

#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>

namespace margot {
namespace heel {

// this function generates the actual high-level interface
cpp_source_content margot_hpp_content(const application_model& app);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_CPP_MARGOT_HDR_HDR