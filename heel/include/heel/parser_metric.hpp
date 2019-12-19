#ifndef HEEL_PARSER_METRIC_HDR
#define HEEL_PARSER_METRIC_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_metric.hpp>

namespace margot {
namespace heel {

void parse(metric_model& metric, const boost::property_tree::ptree& metric_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_METRIC_HDR