#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

#include <heel/model/metric.hpp>
#include <heel/parser/metric.hpp>
#include <heel/parser/utils.hpp>

namespace pt = boost::property_tree;

// this struct is used to store the actual values of a tag
struct tag {
  inline static const std::string metrics(void) { return "metrics"; }
  inline static const std::string name(void) { return "name"; }
  inline static const std::string metric_type(void) { return "type"; }
  inline static const std::string plugin(void) { return "predict_with"; }
  inline static const std::string distribution(void) { return "distribution"; }
};

// forward declaration of the function that actually parse a metric description
margot::heel::metric_model parse_metric_model(const pt::ptree& metric_node);

// this function basically iterates over the metrics defined in the file and call the appropriate function
// to parse it, appending the new metric to the result vector
std::vector<margot::heel::metric_model> margot::heel::parse_metrics(const pt::ptree& block_node) {
  std::vector<margot::heel::metric_model> result;
  margot::heel::visit_optional(tag::metrics(), block_node, [&result](const pt::ptree::value_type& p) {
    result.emplace_back(parse_metric_model(p.second));
  });

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
  std::string is_dist_str = metric_node.get<std::string>(tag::distribution(), "");
  std::transform(is_dist_str.begin(), is_dist_str.end(), is_dist_str.begin(),
                 [](typename std::string::value_type c) { return std::tolower(c); });
  const bool is_distribution = ((is_dist_str.compare("yes") == 0) || (is_dist_str.compare("on") == 0) ||
                                (is_dist_str.compare("true") == 0) || (is_dist_str.compare("1") == 0))
                                   ? true
                                   : false;

  // create the model
  return {metric_node.get<std::string>(tag::name(), ""), metric_node.get<std::string>(tag::metric_type(), ""),
          is_distribution, metric_node.get<std::string>(tag::plugin(), "")};
}
