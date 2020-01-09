#include <algorithm>
#include <utility>

#include <boost/property_tree/ptree.hpp>

#include <heel/composer_monitor.hpp>
#include <heel/composer_parameter.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_monitor.hpp>
#include <heel/parser_tags.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& monitor_node, const monitor_model& monitor) {
  // write the basic information about the monitor
  monitor_node.put(tag::name(), monitor.name);
  monitor_node.put(tag::type(), monitor.type);

  // write all the other information
  add_list(monitor_node, monitor.requested_statistics, tag::log());
  add_list(monitor_node, monitor.initialization_parameters, tag::constructor());
  add_list(monitor_node, monitor.start_parameters, tag::start());
  add_list(monitor_node, monitor.stop_parameters, tag::stop());
}

}  // namespace heel
}  // namespace margot