#ifndef HEEL_PARSER_STATE_HDR
#define HEEL_PARSER_STATE_HDR

#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_state.hpp>

namespace margot {
namespace heel {

std::vector<state_model> parse_states(const boost::property_tree::ptree& block_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_STATE_HDR