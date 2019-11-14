#ifndef HEEL_CPP_STATE_EMITTER_HDR
#define HEEL_CPP_STATE_EMITTER_HDR

#include <string>

#include <heel/generator_utils.hpp>
#include <heel/model_state.hpp>

namespace margot {
namespace heel {

// this function generates the cpp code that initialize the state according to its model
cpp_source_content generate_cpp_content(const state_model& state, const std::string block_name);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CPP_STATE_EMITTER_HDR