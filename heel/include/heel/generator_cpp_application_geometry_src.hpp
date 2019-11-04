#ifndef HEEL_GENERATOR_CPP_APPLICATION_GEOMETRY_SRC_HDR
#define HEEL_GENERATOR_CPP_APPLICATION_GEOMETRY_SRC_HDR

#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>

namespace margot {
namespace heel {

// this function generates the content of the source file that defines the parser of the Operating Point, from
// the string representation to the actual object used by margot
cpp_source_content application_geometry_cpp_content(const application_model& app);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_CPP_APPLICATION_GEOMETRY_SRC_HDR