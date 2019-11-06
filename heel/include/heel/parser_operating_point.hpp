#ifndef HEEL_PARSER_OPERATING_POINT_HDR
#define HEEL_PARSER_OPERATING_POINT_HDR

#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_block.hpp>
#include <heel/model_operating_point.hpp>

namespace margot {
namespace heel {

std::vector<operating_point_model> parse_operating_points(const boost::property_tree::ptree& op_node,
                                                          const block_model& block);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_OPERATING_POINT_HDR