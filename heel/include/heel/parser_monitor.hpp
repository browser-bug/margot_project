#ifndef HEEL_PARSER_MONITOR_HDR
#define HEEL_PARSER_MONITOR_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_monitor.hpp>

namespace margot {
namespace heel {

void parse(monitor_model& monitor, const boost::property_tree::ptree& monitor_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_MONITOR_HDR