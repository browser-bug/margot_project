#include <algorithm>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_monitor.hpp>
#include <heel/parser_monitor.hpp>
#include <heel/parser_parameter.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

void margot::heel::parse(monitor_model& monitor, const boost::property_tree::ptree& monitor_node) {
  margot::heel::parse_element(monitor.name, monitor_node, margot::heel::tag::name());
  margot::heel::parse_element(monitor.type, monitor_node, margot::heel::tag::type());
  margot::heel::parse_list(monitor.requested_statistics, monitor_node, margot::heel::tag::log());
  margot::heel::parse_list(monitor.initialization_parameters, monitor_node, margot::heel::tag::constructor());
  margot::heel::parse_list(monitor.start_parameters, monitor_node, margot::heel::tag::start());
  margot::heel::parse_list(monitor.stop_parameters, monitor_node, margot::heel::tag::stop());
}
