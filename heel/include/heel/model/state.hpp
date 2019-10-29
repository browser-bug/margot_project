#ifndef HEEL_MODEL_STATE_HDR
#define HEEL_MODEL_STATE_HDR

#include <cstdint>
#include <string>
#include <vector>

#include <heel/model/knob.hpp>
#include <heel/model/metric.hpp>

namespace margot {
namespace heel {

enum class goal_comparison { LESS_OR_EQUAL, GREATER_OR_EQUAL, LESS, GREATER };
enum class rank_direction { MINIMIZE, MAXIMIZE, NONE };
enum class rank_type { SIMPLE, GEOMETRIC, LINEAR, NONE };

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

// this function validates a state model to enforce a correct configuration of the application
void validate(state_model& model, const std::vector<metric_model>& metrics,
              const std::vector<knob_model>& knobs);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_STATE_HDR