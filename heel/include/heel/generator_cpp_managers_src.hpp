#ifndef HEEL_GENERATOR_CPP_MANAGERS_SRC_HDR
#define HEEL_GENERATOR_CPP_MANAGERS_SRC_HDR

#include <string>

#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>

namespace margot {
namespace heel {

// this function generates the struct that defines the high-level interface, including the monitors and goals.
cpp_source_content managers_cpp_content(application_model& app, const std::string& app_description);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_CPP_MANAGERS_SRC_HDR