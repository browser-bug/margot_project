#ifndef HEEL_PARSER_AGORA_HDR
#define HEEL_PARSER_AGORA_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_agora.hpp>

namespace margot {
namespace heel {

agora_model parse_agora(const boost::property_tree::ptree& block_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_AGORA_HDR