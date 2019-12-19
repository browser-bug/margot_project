#ifndef HEEL_PARSER_PARAMETER_HDR
#define HEEL_PARSER_PARAMETER_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_parameter.hpp>

namespace margot {
namespace heel {

void parse(parameter& parameter, const boost::property_tree::ptree& parameter_node);
void parse(pair_property& property, const boost::property_tree::ptree& property_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_PARAMETER_HDR