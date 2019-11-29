#ifndef HEEL_COMPOSER_PARAMETER_HDR
#define HEEL_COMPOSER_PARAMETER_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_parameter.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& parameter_node, const margot::heel::parameter& parameter);
void compose(boost::property_tree::ptree& property_node, const pair_property& property);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_COMPOSER_PARAMETER_HDR