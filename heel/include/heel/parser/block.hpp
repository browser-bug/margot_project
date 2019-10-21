#ifndef HEEL_PARSER_BLOCK_HDR
#define HEEL_PARSER_BLOCK_HDR

#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model/block.hpp>

namespace margot {
namespace heel {

std::vector<block_model> parse_blocks(const boost::property_tree::ptree& application_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_BLOCK_HDR