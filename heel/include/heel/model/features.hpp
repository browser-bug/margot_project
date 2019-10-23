#ifndef HEEL_MODEL_FEATURES_HDR
#define HEEL_MODEL_FEATURES_HDR

#include <string>
#include <vector>

namespace margot {
namespace heel {

enum class distance_comparison_type { LESS_OR_EQUAL, GREATER_OR_EQUAL, DONT_CARE };

struct feature_model {
  std::string name;
  std::string type;
  distance_comparison_type comparison;
};

struct features_model {
  std::string distance_type;
  std::vector<feature_model> fields;
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_FEATURES_HDR