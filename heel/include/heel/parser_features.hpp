#ifndef HEEL_PARSER_FEATURES_HDR
#define HEEL_PARSER_FEATURES_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_features.hpp>

namespace margot {
namespace heel {

void parse(feature_model& field, const boost::property_tree::ptree& feature_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_FEATURES_HDR