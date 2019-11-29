#include <algorithm>
#include <utility>

#include <boost/property_tree/ptree.hpp>

#include <heel/composer_monitor.hpp>
#include <heel/composer_parameter.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_monitor.hpp>
#include <heel/parser_tags.hpp>

void margot::heel::compose(boost::property_tree::ptree& monitor_node, const monitor_model& monitor) {
  // write the basic information about the monitor
  monitor_node.put(margot::heel::tag::name(), monitor.name);
  monitor_node.put(margot::heel::tag::type(), monitor.type);

  // write all the other information
  margot::heel::add_list(monitor_node, monitor.requested_statistics, margot::heel::tag::log());
  margot::heel::add_list(monitor_node, monitor.initialization_parameters, margot::heel::tag::constructor());
  margot::heel::add_list(monitor_node, monitor.start_parameters, margot::heel::tag::start());
  margot::heel::add_list(monitor_node, monitor.stop_parameters, margot::heel::tag::stop());
}