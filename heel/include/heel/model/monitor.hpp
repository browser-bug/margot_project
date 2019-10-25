#ifndef HEEL_MODEL_MONITOR_HDR
#define HEEL_MODEL_MONITOR_HDR

#include <array>
#include <string>
#include <vector>

#include <heel/model/parameter.hpp>

namespace margot {
namespace heel {

struct monitor_model {
  std::string name;
  std::string type;

  // NOTE: by convention the name of the statistic must be the same of the related method in the margot
  // statistical provider class
  std::vector<std::string> requested_statistics;  // e.g. average, standard_deviation, ...

  // these are the input parameters required to construct a monitor and to start/stop a measure
  std::vector<struct parameter> initialization_parameters;
  std::vector<struct parameter> start_parameters;
  std::vector<struct parameter> stop_parameters;
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_MONITOR_HDR