#ifndef HEEL_MODEL_STATE_HDR
#define HEEL_MODEL_STATE_HDR

#include <cstdint>
#include <string>
#include <vector>

namespace margot {
namespace heel {

enum class goal_comparison { LESS_OR_EQUAL, GREATER_OR_EQUAL, LESS, GREATER, UNKNOWN };
enum class rank_direction { MINIMIZE, MAXIMIZE, UNKNOWN };
enum class rank_type { SIMPLE, GEOMETRIC, LINEAR, UNKNOWN };

struct constraint_model {
  std::string field_name;
  goal_comparison cfun;
  std::string value;
  std::string confidence;
  std::size_t inertia;  // zero means no reaction
};

struct rank_field_model {
  std::string field_name;
  std::string coefficient;
};

struct state_model {
  std::string name;
  rank_direction direction;
  rank_type combination;
  std::vector<rank_field_model> rank_fields;
  std::vector<constraint_model> constraints;
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_STATE_HDR