#ifndef HEEL_GENERATOR_DESCRIPTION_VERBOSE_HDR
#define HEEL_GENERATOR_DESCRIPTION_VERBOSE_HDR

#include <sstream>

#include <heel/model/monitor.hpp>

namespace margot {
namespace heel {

std::stringstream description_verbose(const monitor_model& model);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_DESCRIPTION_VERBOSE_HDR