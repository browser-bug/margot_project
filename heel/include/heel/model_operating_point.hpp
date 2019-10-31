#ifndef HEEL_MODEL_OPERATING_POINT_HDR
#define HEEL_MODEL_OPERATING_POINT_HDR

#include <string>
#include <vector>

namespace margot {
namespace heel {

struct operating_point_value {
  std::string mean;
  std::string standard_deviation;
};

struct operating_point_model {
  std::vector<operating_point_value> features;
  std::vector<operating_point_value> knobs;
  std::vector<operating_point_value> metrics;
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_OPERATING_POINT_HDR