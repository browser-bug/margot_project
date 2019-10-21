#ifndef HEEL_PARSER_APPLICATION_HDR
#define HEEL_PARSER_APPLICATION_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model/application.hpp>

namespace margot {
namespace heel {

application_model parse_application(const boost::property_tree::ptree& application_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_APPLICATION_HDR