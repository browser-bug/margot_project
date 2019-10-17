#ifndef HEEL_PARSER_PARAMETER_HDR
#define HEEL_PARSER_PARAMETER_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/parameters.hpp>

namespace margot {
namespace heel {

// to lower the verbosity of defining a parameter, we need the pair "name" "value" (where value == ptree)
margot::heel::parameter parse_parameter(const boost::property_tree::ptree::value_type& parameter_pair);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_PARAMETER_HDR