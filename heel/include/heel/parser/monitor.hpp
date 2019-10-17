#ifndef HEEL_PARSER_MONITOR_HDR
#define HEEL_PARSER_MONITOR_HDR

#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model/monitor.hpp>

namespace margot {
namespace heel {

std::vector<monitor_model> parse_monitors(const boost::property_tree::ptree& block_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_MONITOR_HDR