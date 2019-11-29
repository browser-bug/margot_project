#ifndef HEEL_COMPOSER_METRIC_HDR
#define HEEL_COMPOSER_METRIC_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_metric.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& metric_node, const metric_model& metric);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_COMPOSER_METRIC_HDR