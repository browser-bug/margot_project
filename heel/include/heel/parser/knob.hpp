#ifndef HEEL_PARSER_KNOB_HDR
#define HEEL_PARSER_KNOB_HDR

#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model/knob.hpp>

namespace margot {
namespace heel {

std::vector<knob_model> parse_knobs(const boost::property_tree::ptree& block_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_KNOB_HDR