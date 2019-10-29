#ifndef HEEL_MODEL_MONITOR_HDR
#define HEEL_MODEL_MONITOR_HDR

#include <array>
#include <string>
#include <vector>

#include <heel/model_parameter.hpp>

namespace margot {
namespace heel {

// this struct holds information about the monitor types and usage
struct monitor_spec {
  std::string class_name;
  std::string header_name;
  std::string value_type;
  std::string start_method_name;
  std::string stop_method_name;
  std::vector<struct parameter> default_param_initialization;
  std::vector<struct parameter> default_param_start;
  std::vector<struct parameter> default_param_stop;
};

// this is the actual monitor model, with all the parameters
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

// this function validates a monitor model; i.e. it checks if there are missing or wrong information
void validate(monitor_model& model);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_MONITOR_HDR