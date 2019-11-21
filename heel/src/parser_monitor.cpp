#include <algorithm>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_monitor.hpp>
#include <heel/parser_monitor.hpp>
#include <heel/parser_parameter.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

// forward declaration of the function that actually parse a monitor description
margot::heel::monitor_model parse_monitor_model(const pt::ptree& monitor_node);

// this function basically iterates over the monitors defined in the file and call the appropriate function
// to parse it, appending the new monitor to the result vector
std::vector<margot::heel::monitor_model> margot::heel::parse_monitors(const pt::ptree& block_node) {
  std::vector<margot::heel::monitor_model> result;
  margot::heel::visit_optional(
      margot::heel::tag::monitors(), block_node,
      [&result](const pt::ptree::value_type& p) { result.emplace_back(parse_monitor_model(p.second)); });
  return result;
}

// this is the main function that actually parse a monitor
margot::heel::monitor_model parse_monitor_model(const pt::ptree& monitor_node) {
  // initialize the model of this new monitor
  margot::heel::monitor_model model = {margot::heel::get(margot::heel::tag::name(), monitor_node),
                                       margot::heel::get(margot::heel::tag::type(), monitor_node),
                                       {},
                                       {},
                                       {},
                                       {}};

  // parse the output variables of a monitor (if any)
  margot::heel::visit_optional(
      margot::heel::tag::log(), monitor_node, [&model](const pt::ptree::value_type& p) {
        model.requested_statistics.emplace_back(p.second.get<std::string>("", ""));  // we just need its name
        // make its name lowercase
        std::transform(model.requested_statistics.back().begin(), model.requested_statistics.back().end(),
                       model.requested_statistics.back().begin(),
                       [](unsigned char c) { return std::tolower(c); });
      });

  // parse the monitor constructor parameters (if any)
  margot::heel::visit_optional(
      margot::heel::tag::constructor(), monitor_node, [&model](const pt::ptree::value_type& p) {
        model.initialization_parameters.emplace_back(margot::heel::parse_parameter(p));
      });

  // parse the monitor start parameters (if any)
  margot::heel::visit_optional(margot::heel::tag::start(), monitor_node,
                               [&model](const pt::ptree::value_type& p) {
                                 model.start_parameters.emplace_back(margot::heel::parse_parameter(p));
                               });

  // parse the monitor stop parameters (if any)
  margot::heel::visit_optional(margot::heel::tag::stop(), monitor_node,
                               [&model](const pt::ptree::value_type& p) {
                                 model.stop_parameters.emplace_back(margot::heel::parse_parameter(p));
                               });

  return model;
}