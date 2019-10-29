#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include <heel/logger.hpp>
#include <heel/model/state.hpp>

void margot::heel::validate(state_model& model, const std::vector<metric_model>& metrics,
                            const std::vector<knob_model>& knobs) {
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
    if (field.field_name.empty()) {
      margot::heel::error("Empty field name in the rank definition for state \"", model.name, "\"");
      throw std::runtime_error("state model: empty rank field");
    }
    if (field.coefficient.empty()) {
      margot::heel::warning("Automatically set \"1\" to coefficient of field \"", field.field_name,
                            "\" in state \"", model.name, "\"");
      field.coefficient = "1";
    }
  });

  // next, we need to validate all the information on the constraint models of a state
  std::for_each(model.constraints.begin(), model.constraints.end(), [&model](constraint_model& constraint) {
    if (constraint.field_name.empty()) {
      margot::heel::error("Empty field name in a constraint for state \"", model.name, "\"");
      throw std::runtime_error("state model: empty constraint field");
    }
    if (constraint.value.empty()) {
      margot::heel::error("The constraint on \"", constraint.field_name, "\" in state \"", model.name,
                          "\" has no goal value");
      throw std::runtime_error("state model: no goal value");
    }
    if (constraint.confidence.empty()) {  // we don't warn the user because is a well defined default value
      constraint.confidence = "0";
    }
  });

  // to ease the cross-checks we define a lambda to check if a field exists
  const auto check_field = [&metrics, &knobs](const std::string& field_name) {
    if (std::none_of(metrics.begin(), metrics.end(), [&field_name](const metric_model& metric) {
          return metric.name.compare(field_name) == 0;
        })) {
      if (std::none_of(knobs.begin(), knobs.end(), [&field_name](const knob_model& knob) {
            return knob.name.compare(field_name) == 0;
          })) {
        // the field is not a metric or a knob, we can't fix that
        margot::heel::error("The field \"", field_name, "\" is not a metric nor a knob");
        throw std::runtime_error("state model: unknown field");
      }
    }
  };

  // the first cross-checks aims at enforcing that a field in a rank or constraint is known
  std::for_each(model.rank_fields.begin(), model.rank_fields.end(),
                [&check_field](const rank_field_model& r) { check_field(r.field_name); });
  std::for_each(model.constraints.begin(), model.constraints.end(),
                [&check_field](const constraint_model& c) { check_field(c.field_name); });

  // finally, we need to ensure that if we want to react against a metric, it has to be observed at runtime
  std::for_each(model.constraints.begin(), model.constraints.end(), [&metrics](const constraint_model& c) {
    if (c.inertia > 0) {
      if (std::none_of(metrics.begin(), metrics.end(), [&c](const metric_model& metric) {
            return (metric.name.compare(c.field_name) == 0) && (!metric.monitor_name.empty());
          })) {
        margot::heel::error("The constraint on \"", c.field_name,
                            "\" is reactive, but there are no observation at runtime on that field");
      }
    }
  });
}
