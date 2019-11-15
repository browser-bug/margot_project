#ifndef HEEL_GENERATOR_CPP_KNOWLEDGE_SRC_HDR
#define HEEL_GENERATOR_CPP_KNOWLEDGE_SRC_HDR

#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>

namespace margot {
namespace heel {

// this function generates the header which define the function that will provide the application knowledge
// (if an Operating Point list is provided).
cpp_source_content knowledge_cpp_content(application_model& app);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_CPP_KNOWLEDGE_SRC_HDR