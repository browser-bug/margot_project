#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

#include <heel/model_metric.hpp>
#include <heel/parser_metric.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

// forward declaration of the function that actually parse a metric description
margot::heel::metric_model parse_metric_model(const pt::ptree& metric_node);

// this function basically iterates over the metrics defined in the file and call the appropriate function
// to parse it, appending the new metric to the result vector
std::vector<margot::heel::metric_model> margot::heel::parse_metrics(const pt::ptree& block_node) {
  std::vector<margot::heel::metric_model> result;
  margot::heel::visit_optional(
      margot::heel::tag::metrics(), block_node,
      [&result](const pt::ptree::value_type& p) { result.emplace_back(parse_metric_model(p.second)); });

  // the list might be full of metric models, but it is better to sort them according to the metric's name
  std::sort(result.begin(), result.end(),
            [](const margot::heel::metric_model& a, const margot::heel::metric_model& b) {
              return a.name < b.name;
            });
  return result;
}

// this is the main function that actually parse a metric
margot::heel::metric_model parse_metric_model(const pt::ptree& metric_node) {
  // the only component that could be a problem is the check if this metric is a distribution. Therefore, we
  // need to check against several ways of expressing yes
  std::string is_dist_str = margot::heel::get(margot::heel::tag::distribution(), metric_node);
  std::transform(is_dist_str.begin(), is_dist_str.end(), is_dist_str.begin(),
                 [](typename std::string::value_type c) { return std::tolower(c); });
  const bool is_distribution = ((is_dist_str.compare("yes") == 0) || (is_dist_str.compare("on") == 0) ||
                                (is_dist_str.compare("true") == 0) || (is_dist_str.compare("1") == 0))
                                   ? true
                                   : false;

  // get the type of the metric and remove the std:: prefix, since it generates some problem with the c
  // interface that we are going to generate
  std::string metric_type = margot::heel::get(margot::heel::tag::type(), metric_node);
  if (metric_type.rfind("std::", 0) == 0) {  // remove any std:: prefix from the type
    metric_type.erase(0, 5);
  }

  // get the reactive inertia as a string, to figure out later if we really need to react
  const std::string inertia_str = margot::heel::get(margot::heel::tag::reactive_inertia(), metric_node);

  // create a default metric model
  margot::heel::metric_model model = {
      margot::heel::get(margot::heel::tag::name(), metric_node),
      metric_type,
      is_distribution,
      margot::heel::get(margot::heel::tag::prediction_plugin(), metric_node),
      margot::heel::get(margot::heel::tag::observed_by(), metric_node),
      !inertia_str.empty() ? boost::lexical_cast<std::size_t>(inertia_str) : 0};

  // parse the parameters for the plugin that will predict its value (if using agora)
  margot::heel::visit_optional(margot::heel::tag::prediction_params(), metric_node,
                               [&model](const pt::ptree::value_type& p) {
                                 model.prediction_parameters.emplace_back(
                                     margot::heel::pair_property{p.first, p.second.get<std::string>("", "")});
                               });

  // create the model
  return model;
}
