#ifndef HEEL_COMPOSER_APPLICATION_HDR
#define HEEL_COMPOSER_APPLICATION_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_application.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& root_node, const application_model& app);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_COMPOSER_APPLICATION_HDR