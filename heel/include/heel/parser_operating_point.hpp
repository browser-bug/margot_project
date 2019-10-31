#ifndef HEEL_PARSER_OPERATING_POINT_HDR
#define HEEL_PARSER_OPERATING_POINT_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_application.hpp>

namespace margot {
namespace heel {

void parse_operating_points(const boost::property_tree::ptree& op_node, application_model& application);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_OPERATING_POINT_HDR