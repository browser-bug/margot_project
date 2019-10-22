#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>

#include <heel/model/state.hpp>
#include <heel/parser/state.hpp>
#include <heel/parser/utils.hpp>

namespace pt = boost::property_tree;

// this struct is used to store the actual values of a tag
struct tag {
  inline static const std::string states(void) { return "extra-functional_requirements"; }
  inline static const std::string name(void) { return "name"; }
  inline static const std::string maximize(void) { return "maximize"; }
  inline static const std::string minimize(void) { return "minimize"; }
  inline static const std::string geometric_mean(void) { return "geometric_mean"; }
  inline static const std::string linear_mean(void) { return "linear_mean"; }
  inline static const std::string constraints(void) { return "subject_to"; }
  inline static const std::string subject(void) { return "subject"; }
  inline static const std::string comparison(void) { return "comparison"; }
  inline static const std::string value(void) { return "value"; }
  inline static const std::string confidence(void) { return "confidence"; }
  inline static const std::string reactive_inertia(void) { return "reactive_inertia"; }
};

// forward declaration of the functions that parse a portion of a state
margot::heel::state_model parse_state_model(const pt::ptree& block_node);
margot::heel::constraint_model parse_constraint_model(const pt::ptree& constraint_node);

// this function basically iterates over the states defined in the file and call the appropriate function
// to parse it, appending the new state to the result vector.
std::vector<margot::heel::state_model> margot::heel::parse_states(
    const boost::property_tree::ptree& block_node) {
  std::vector<margot::heel::state_model> result;
  margot::heel::visit_optional(tag::states(), block_node, [&result](const pt::ptree::value_type& p) {
    result.emplace_back(parse_state_model(p.second));
  });
  return result;
}

margot::heel::state_model parse_state_model(const pt::ptree& state_node) {
  // declare the default state model
  margot::heel::state_model model = {margot::heel::get(tag::name(), state_node),
                                     margot::heel::rank_direction::MAXIMIZE,
                                     margot::heel::rank_type::SIMPLE,
                                     {},
                                     {}};

  // define a lambda to extract all the rank fields from either the maximization or minimization node
  const auto extract_fields = [&model](const pt::ptree& rank_node) {
    // parse all the fields of the rank, assuming that it is a geometric mean
    margot::heel::visit_optional(tag::geometric_mean(), rank_node, [&model](const pt::ptree::value_type& p) {
      model.rank_fields.emplace_back(
          margot::heel::rank_field_model{p.first, p.second.get<std::string>("", "")});
    });
    model.combination = margot::heel::rank_type::GEOMETRIC;

    // if they are empty, try with the linear combination
    if (model.rank_fields.empty()) {
      margot::heel::visit_optional(tag::linear_mean(), rank_node, [&model](const pt::ptree::value_type& p) {
        model.rank_fields.emplace_back(
            margot::heel::rank_field_model{p.first, p.second.get<std::string>("", "")});
      });
      model.combination = margot::heel::rank_type::LINEAR;
    }

    // check if we need to combine different fields, or we can use directly a single entry
    if (model.rank_fields.size() < 2) {
      model.combination = margot::heel::rank_type::SIMPLE;
    }
  };

  // check if we need a minimization or a maximization rank
  const auto minimize_node = state_node.get_child_optional(tag::minimize());
  if (minimize_node) {
    model.direction = margot::heel::rank_direction::MINIMIZE;
    extract_fields(*minimize_node);
  } else {
    const auto maximize_node = state_node.get_child_optional(tag::maximize());
    if (maximize_node) {
      model.direction = margot::heel::rank_direction::MAXIMIZE;
      extract_fields(*maximize_node);
    }
  }

  // parse all the constraints
  margot::heel::visit_optional(tag::constraints(), state_node, [&model](const pt::ptree::value_type& p) {
    model.constraints.emplace_back(parse_constraint_model(p.second));
  });

  return model;
}

margot::heel::constraint_model parse_constraint_model(const pt::ptree& constraint_node) {
  // figure out the type of comparison function
  std::string comparison_fun_str = margot::heel::get(tag::comparison(), constraint_node);
  std::transform(comparison_fun_str.begin(), comparison_fun_str.end(), comparison_fun_str.begin(),
                 [](typename std::string::value_type c) { return std::tolower(c); });
  margot::heel::goal_comparison cfun = margot::heel::goal_comparison::LESS_OR_EQUAL;
  if (comparison_fun_str.compare("lt") == 0) {
    cfun = margot::heel::goal_comparison::LESS;
  } else if (comparison_fun_str.compare("ge") == 0) {
    cfun = margot::heel::goal_comparison::GREATER_OR_EQUAL;
  } else if (comparison_fun_str.compare("gt") == 0) {
    cfun = margot::heel::goal_comparison::GREATER;
  }

  // get the reactive inertia as a string, to figure out later if we really need to react
  const std::string inertia_str = margot::heel::get(tag::reactive_inertia(), constraint_node);

  // reached this point, we can compose the model
  return {margot::heel::get(tag::subject(), constraint_node), cfun,
          margot::heel::get(tag::value(), constraint_node),
          margot::heel::get(tag::confidence(), constraint_node),
          !inertia_str.empty() ? boost::lexical_cast<std::size_t>(inertia_str) : 0};
}