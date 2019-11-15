#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include <heel/logger.hpp>
#include <heel/model_state.hpp>
#include <heel/typer.hpp>

void margot::heel::validate(state_model& model, const std::vector<metric_model>& metrics,
                            const std::vector<knob_model>& knobs) {
  // at first we need to be sure that a state has a name and that it is a valid c++ identifier
  if (model.name.empty()) {
    margot::heel::error("The name of a state is empty");
    throw std::runtime_error("state model: empty state name");
  }
  if (!margot::heel::is_valid_identifier(model.name)) {
    margot::heel::error("The name of state \"", model.name, "\" is not a valid C/C++ identifier");
    throw std::runtime_error("state model: invalid state name");
  }

  // make sure that if the user defined rank fields, the other parameters of the rank are correct
  if (!model.rank_fields.empty()) {
    if (model.direction == margot::heel::rank_direction::NONE) {
      margot::heel::error("The rank direction (i.e. maximize/minimize) is not set");
      throw std::runtime_error("state model: missing rank direction");
    }
    if (model.combination == margot::heel::rank_type::NONE) {
      margot::heel::error("The rank type (i.e. geometric_mean/linear_mean) is not set");
      throw std::runtime_error("state model: missing rank type");
    }
  } else {
    margot::heel::warning("The rank for state \"", model.name, "\" is undefined");
  }

  // now, we have to unsure that the coefficient of each fild of the rank is set
  std::for_each(model.rank_fields.begin(), model.rank_fields.end(), [&model](rank_field_model& field) {
    if (field.name.empty()) {
      margot::heel::error("Empty field name in the rank definition for state \"", model.name, "\"");
      throw std::runtime_error("state model: empty rank field");
    }
    if (field.coefficient.empty()) {
      margot::heel::warning("Automatically set \"1\" to coefficient of field \"", field.name,
                            "\" in state \"", model.name, "\"");
      field.coefficient = "1";
    }
  });

  // next, we need to validate all the information on the constraint models of a state
  std::for_each(model.constraints.begin(), model.constraints.end(), [&model](constraint_model& constraint) {
    if (constraint.name.empty()) {
      margot::heel::error("Empty field name in a constraint for state \"", model.name, "\"");
      throw std::runtime_error("state model: empty constraint field");
    }
    if (constraint.value.empty()) {
      margot::heel::error("The constraint on \"", constraint.name, "\" in state \"", model.name,
                          "\" has no goal value");
      throw std::runtime_error("state model: no goal value");
    }
    if (constraint.confidence.empty()) {  // we don't warn the user because is a well defined default value
      constraint.confidence = "0";
    }
  });

  // to ease the cross-checks we define a lambda to check if a field exists
  const auto is_a_metric = [&metrics](const std::string& name) {
    if (std::any_of(metrics.begin(), metrics.end(),
                    [&name](const metric_model& metric) { return metric.name.compare(name) == 0; })) {
      return true;
    } else {
      return false;
    }
  };
  const auto is_a_knob = [&knobs](const std::string& name) {
    if (std::any_of(knobs.begin(), knobs.end(),
                    [&name](const knob_model& knob) { return knob.name.compare(name) == 0; })) {
      return true;
    } else {
      return false;
    }
  };

  // the first cross-checks aim at enforcing that a field in a rank or constraint is known
  std::for_each(model.rank_fields.begin(), model.rank_fields.end(),
                [&is_a_metric, &is_a_knob](rank_field_model& r) {
                  if (is_a_knob(r.name)) {
                    r.kind = margot::heel::subject_kind::KNOB;
                  } else if (is_a_metric(r.name)) {
                    r.kind = margot::heel::subject_kind::METRIC;
                  } else {
                    margot::heel::error("The rank field \"", r.name, "\" is not metric nor a knob");
                    throw std::runtime_error("state model: unknown rank field");
                  }
                });
  std::for_each(
      model.constraints.begin(), model.constraints.end(), [&is_a_metric, &is_a_knob](constraint_model& c) {
        if (is_a_knob(c.name)) {
          c.kind = margot::heel::subject_kind::KNOB;
        } else if (is_a_metric(c.name)) {
          c.kind = margot::heel::subject_kind::METRIC;
        } else {
          margot::heel::error("The constraint on field \"", c.name, "\" is not on a metric nor a nob");
          throw std::runtime_error("state model: unknown constraint field");
        }
      });
}
