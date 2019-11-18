#ifndef HEEL_GENERATOR_CPP_MANAGERS_HDR_HDR
#define HEEL_GENERATOR_CPP_MANAGERS_HDR_HDR

#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>

namespace margot {
namespace heel {

// this function generates the struct that defines the high-level interface, including the monitors and goals.
cpp_source_content managers_hpp_content(application_model& app);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_CPP_MANAGERS_HDR_HDR