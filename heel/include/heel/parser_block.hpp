#ifndef HEEL_PARSER_BLOCK_HDR
#define HEEL_PARSER_BLOCK_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_block.hpp>

namespace margot {
namespace heel {

void parse(block_model& block, const boost::property_tree::ptree& block_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_BLOCK_HDR