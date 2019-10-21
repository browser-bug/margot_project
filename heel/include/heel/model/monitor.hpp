#ifndef HEEL_MODEL_MONITOR_HDR
#define HEEL_MODEL_MONITOR_HDR

#include <array>
#include <string>
#include <vector>

#include <heel/model/parameter.hpp>

namespace margot {
namespace heel {

struct monitor_spec {
  std::string class_name;
  std::string header_path;
  std::string value_type;
  std::string start_method_name;
  std::string stop_method_name;
};

struct monitor_model {
  std::string cpp_identifier;  // aka the monitor name
  monitor_spec spec;

  // NOTE: by convention the name of the statistic must be the same of the related method in the margot
  // statistical provider class
  std::vector<std::string> requested_statistics;  // e.g. average, standard_deviation, ...

  // these are the input parameters required to construct a monitor and to start/stop a measure
  std::vector<struct parameter> initialization_parameters;
  std::vector<struct parameter> start_parameters;
  std::vector<struct parameter> stop_parameters;
};

monitor_model create_monitor(const std::string& monitor_type);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_MONITOR_HDR