#ifndef HEEL_COMPOSER_STATE_HDR
#define HEEL_COMPOSER_STATE_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_state.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& state_node, const state_model& state);
void compose(boost::property_tree::ptree& rank_node, const rank_field_model& rank);
void compose(boost::property_tree::ptree& constraint_node, const constraint_model& constraint);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_COMPOSER_STATE_HDR