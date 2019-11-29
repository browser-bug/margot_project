#ifndef HEEL_COMPOSER_FEATURE_FIELDS_HDR
#define HEEL_COMPOSER_FEATURE_FIELDS_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_features.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& field_node, const feature_model& field);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_COMPOSER_FEATURE_FIELDS_HDR