#ifndef HEEL_PARSER_METRIC_HDR
#define HEEL_PARSER_METRIC_HDR

#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model/metric.hpp>

namespace margot {
namespace heel {

std::vector<metric_model> parse_metrics(const boost::property_tree::ptree& block_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_METRIC_HDR