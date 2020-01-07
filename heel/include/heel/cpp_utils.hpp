#ifndef HEEL_CPP_UTILS_HDR
#define HEEL_CPP_UTILS_HDR

#include <cstdint>
#include <string>

#include <heel/model_state.hpp>

namespace margot {
namespace heel {

// this helper function generate the cpp code that translate a field "name" to its index
inline std::string generate_field_getter(const subject_kind kind, const std::string& field_name,
                                         const std::string& block_name) {
  const std::string section_name = kind == subject_kind::METRIC ? std::string("metric") : std::string("knob");
  return "static_cast<size_t>(margot::" + block_name + "::" + section_name + "::" + field_name + ")";
}

// this function generates the goal cpp identifier that must be used in the generated code
inline std::string generate_goal_identifier(const std::string& state_name, const std::size_t id) {
  return state_name + "_constraint_" + std::to_string(id);
}

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CPP_UTILS_HDR