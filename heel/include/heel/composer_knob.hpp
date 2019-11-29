#ifndef HEEL_COMPOSER_KNOB_HDR
#define HEEL_COMPOSER_KNOB_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_knob.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& knob_node, const knob_model& knob);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_COMPOSER_KNOB_HDR