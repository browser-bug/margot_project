#ifndef HEEL_PARSER_OPERATING_POINT_HDR
#define HEEL_PARSER_OPERATING_POINT_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_block.hpp>
#include <heel/model_operating_point.hpp>

namespace margot {
namespace heel {

// NB: this function is different with respect to the other ones, because it needs to use the application
// knowledge to parse the operating point. In this way it can automatically validate their content
void parse_operating_points(block_model& block, const boost::property_tree::ptree& op_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_OPERATING_POINT_HDR