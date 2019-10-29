#ifndef HEEL_PARSER_FEATURES_HDR
#define HEEL_PARSER_FEATURES_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_features.hpp>

namespace margot {
namespace heel {

features_model parse_features(const boost::property_tree::ptree& block_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_FEATURES_HDR