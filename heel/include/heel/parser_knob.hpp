#ifndef HEEL_PARSER_KNOB_HDR
#define HEEL_PARSER_KNOB_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_knob.hpp>

namespace margot {
namespace heel {

void parse(knob_model& knob, const boost::property_tree::ptree& knob_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_KNOB_HDR