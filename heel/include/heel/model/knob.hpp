#ifndef HEEL_MODEL_KNOB_HDR
#define HEEL_MODEL_KNOB_HDR

#include <string>
#include <vector>

namespace margot {
namespace heel {

struct knob_model {
  std::string name;
  std::string type;
  std::vector<std::string> values;
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_KNOB_HDR