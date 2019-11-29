#ifndef HEEL_MODEL_FEATURES_HDR
#define HEEL_MODEL_FEATURES_HDR

#include <string>
#include <vector>

namespace margot {
namespace heel {

enum class features_distance_type { EUCLIDEAN, NORMALIZED, NONE };
enum class distance_comparison_type { LESS_OR_EQUAL, GREATER_OR_EQUAL, DONT_CARE };

struct feature_model {
  std::string name;
  std::string type;
  distance_comparison_type comparison;
};

struct features_model {
  features_distance_type distance_type;
  std::string features_type;
  std::vector<feature_model> fields;
  inline bool empty(void) const { return fields.empty(); }
};

// helper function to translate a feature_distance_type to a string
inline std::string to_str(const margot::heel::features_distance_type type) {
  switch (type) {
    case margot::heel::features_distance_type::EUCLIDEAN:
      return "euclidean";
    case margot::heel::features_distance_type::NORMALIZED:
      return "normalized";
    case margot::heel::features_distance_type::NONE:
      return "none";
    default:
      return "defensive programming:  unknown distance type";
  };
}

// helper function to translate a distance_compare_type to a string
inline std::string to_str(const margot::heel::distance_comparison_type type) {
  switch (type) {
    case margot::heel::distance_comparison_type::LESS_OR_EQUAL:
      return "le";
    case margot::heel::distance_comparison_type::GREATER_OR_EQUAL:
      return "ge";
    case margot::heel::distance_comparison_type::DONT_CARE:
      return "-";
    default:
      return "defensive programming:  unknown comparison type";
  };
}

// this function validates a features model to enforce a correct configuration of the application
void validate(features_model& model);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_FEATURES_HDR