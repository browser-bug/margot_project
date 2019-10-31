#ifndef HEEL_GENERATOR_CPP_APPLICATION_GEOMETRY_HDR
#define HEEL_GENERATOR_CPP_APPLICATION_GEOMETRY_HDR

#include <sstream>

#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>

namespace margot {
namespace heel {

// this function generates the content of the header that defines, for each block, the geometry of the
// operating points, enums to retrieve a value for the software knob, and the functor that parse an Operating
// Point (if agora is enabled)
cpp_source_content application_geometry_hpp_content(const application_model& app);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_CPP_APPLICATION_GEOMETRY_HDR