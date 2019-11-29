#ifndef HEEL_COMPOSER_AGOR_HDR
#define HEEL_COMPOSER_AGOR_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_agora.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& agora_node, const agora_model& agora);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_COMPOSER_AGOR_HDR