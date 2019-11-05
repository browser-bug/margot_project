#include <string>
#include <vector>

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

#include <heel/model_knob.hpp>
#include <heel/parser_knob.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

// this struct is used to store the actual values of a tag
struct tag {
  inline static const std::string knobs(void) { return "knobs"; }
  inline static const std::string name(void) { return "name"; }
  inline static const std::string knob_type(void) { return "type"; }
  inline static const std::string values(void) { return "values"; }
  inline static const std::string range(void) { return "range"; }
};

// forward declaration of the function that actually parse a knob description
margot::heel::knob_model parse_knob_model(const pt::ptree& knob_node);

// this function basically iterates over the knobs defined in the file and call the appropriate function
// to parse it, appending the new knob to the result vector
std::vector<margot::heel::knob_model> margot::heel::parse_knobs(const pt::ptree& block_node) {
  std::vector<margot::heel::knob_model> result;
  margot::heel::visit_optional(tag::knobs(), block_node, [&result](const pt::ptree::value_type& p) {
    result.emplace_back(parse_knob_model(p.second));
  });

  // the list might be full of knob models, but it is better to sort them according to the knob's name
  std::sort(
      result.begin(), result.end(),
      [](const margot::heel::knob_model& a, const margot::heel::knob_model& b) { return a.name < b.name; });
  return result;
}

// this is the main function that actually parse a knob
margot::heel::knob_model parse_knob_model(const pt::ptree& knob_node) {
  // initialize the knob model
  margot::heel::knob_model model = {
      margot::heel::get(tag::name(), knob_node), margot::heel::get(tag::knob_type(), knob_node), {}};

  // parse the list of values (if any) as an array of strings
  margot::heel::visit_optional(tag::values(), knob_node, [&model](const pt::ptree::value_type& p) {
    model.values.emplace_back(p.second.get<std::string>("", ""));
  });

  // if we have them it's ok, otherwise we need to generate them from a range of values
  if (model.values.empty()) {
    const boost::optional<const boost::property_tree::ptree&> range_node =
        knob_node.get_child_optional(tag::range());
    if (range_node) {
      model.values = margot::heel::compute_range(*range_node, model.type);
    }
  }

  return model;
}