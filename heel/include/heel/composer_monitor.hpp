#ifndef HEEL_COMPOSER_MONITOR_HDR
#define HEEL_COMPOSER_MONITOR_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_monitor.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& monitor_node, const monitor_model& monitor);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_COMPOSER_MONITOR_HDR