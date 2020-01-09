#include <algorithm>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_monitor.hpp>
#include <heel/parser_monitor.hpp>
#include <heel/parser_parameter.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace margot {
namespace heel {

void parse(monitor_model& monitor, const boost::property_tree::ptree& monitor_node) {
  parse_element(monitor.name, monitor_node, tag::name());
  parse_element(monitor.type, monitor_node, tag::type());
  parse_list(monitor.requested_statistics, monitor_node, tag::log());
  parse_list(monitor.initialization_parameters, monitor_node, tag::constructor());
  parse_list(monitor.start_parameters, monitor_node, tag::start());
  parse_list(monitor.stop_parameters, monitor_node, tag::stop());
}

}  // namespace heel
}  // namespace margot