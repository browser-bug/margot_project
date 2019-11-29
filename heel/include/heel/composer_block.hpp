#ifndef HEEL_COMPOSER_BLOCK_HDR
#define HEEL_COMPOSER_BLOCK_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_block.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& block_node, const block_model& block);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_COMPOSER_BLOCK_HDR