#ifndef HEEL_PARSER_STATE_HDR
#define HEEL_PARSER_STATE_HDR

#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_state.hpp>

namespace margot {
namespace heel {

void parse(state_model& state, const boost::property_tree::ptree& state_node);
void parse(rank_field_model& rank_field, const boost::property_tree::ptree& rank_field_node);
void parse(constraint_model& constraint, const boost::property_tree::ptree& constraint_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_STATE_HDR