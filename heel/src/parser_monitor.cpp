#include <stdexcept>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model/monitor.hpp>
#include <heel/parser/monitor.hpp>
#include <heel/parser/parameter.hpp>
#include <heel/parser/utils.hpp>

namespace pt = boost::property_tree;

// this struct is used to store the actual values of a tag
struct tag {
  inline static const std::string monitors(void) { return "monitors"; }
  inline static const std::string name(void) { return "name"; }
  inline static const std::string monitor_type(void) { return "type"; }
  inline static const std::string value_type(void) { return "name"; }
  inline static const std::string log(void) { return "log"; }
  inline static const std::string constructor(void) { return "constructor"; }
  inline static const std::string start(void) { return "start"; }
  inline static const std::string stop(void) { return "stop"; }
};

// forward declaration of the function that actually parse a monitor description
margot::heel::monitor_model parse_monitor_model(const pt::ptree& monitor_node);

// this function basically iterates over the monitors defined in the file and call the appropriate function
// to parse it, appending the new monitor to the result vector
std::vector<margot::heel::monitor_model> margot::heel::parse_monitors(const pt::ptree& block_node) {
  std::vector<margot::heel::monitor_model> result;
  margot::heel::visit_optional(tag::monitors(), block_node, [&result](const pt::ptree::value_type& p) {
    result.emplace_back(parse_monitor_model(p.second));
  });
  return result;
}

// this is the main function that actually parse a monitor
margot::heel::monitor_model parse_monitor_model(const pt::ptree& monitor_node) {
  // initialize the model of this new monitor
  margot::heel::monitor_model model =
      margot::heel::create_monitor(monitor_node.get<std::string>(tag::monitor_type(), ""));
  model.cpp_identifier = monitor_node.get<std::string>(tag::name(), "");

  // parse the output variables of a monitor (if any)
  margot::heel::visit_optional(tag::log(), monitor_node, [&model](const pt::ptree::value_type& p) {
    model.requested_statistics.emplace_back(p.first);  // we just need its name
  });

  // parse the monitor constructor parameters (if any)
  margot::heel::visit_optional(tag::constructor(), monitor_node, [&model](const pt::ptree::value_type& p) {
    model.initialization_parameters.emplace_back(margot::heel::parse_parameter(p));
  });

  // parse the monitor start parameters (if any)
  margot::heel::visit_optional(tag::start(), monitor_node, [&model](const pt::ptree::value_type& p) {
    model.start_parameters.emplace_back(margot::heel::parse_parameter(p));
  });

  // parse the monitor stop parameters (if any)
  margot::heel::visit_optional(tag::stop(), monitor_node, [&model](const pt::ptree::value_type& p) {
    model.stop_parameters.emplace_back(margot::heel::parse_parameter(p));
  });

  return model;
}