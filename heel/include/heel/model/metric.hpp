#ifndef HEEL_MODEL_METRIC_HDR
#define HEEL_MODEL_METRIC_HDR

#include <string>

namespace margot {
namespace heel {

struct metric_model {
  std::string name;
  std::string type;
  bool distribution;
  std::string prediction_method;
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_METRIC_HDR