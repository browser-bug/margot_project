/* mARGOt HEEL library
 * Copyright (C) 2018 Politecnico di Milano
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include <heel/logger.hpp>
#include <heel/model_state.hpp>
#include <heel/typer.hpp>

namespace margot {
namespace heel {

void validate(state_model& model, const std::vector<metric_model>& metrics,
              const std::vector<knob_model>& knobs) {
  // at first we need to be sure that a state has a name and that it is a valid c++ identifier
  if (model.name.empty()) {
    error("The name of a state is empty");
    throw std::runtime_error("state model: empty state name");
  }
  if (!is_valid_identifier(model.name)) {
    error("The name of state \"", model.name, "\" is not a valid C/C++ identifier");
    throw std::runtime_error("state model: invalid state name");
  }

  // make sure that if the user defined rank fields, the other parameters of the rank are correct
  if (!model.rank_fields.empty()) {
    if (model.direction == rank_direction::NONE) {
      error("The rank direction (i.e. maximize/minimize) is not set");
      throw std::runtime_error("state model: missing rank direction");
    }
    if (model.combination == rank_type::NONE) {
      error("The rank type (i.e. geometric_mean/linear_mean) is not set");
      throw std::runtime_error("state model: missing rank type");
    }
  } else {
    warning("The rank for state \"", model.name, "\" is undefined");
  }

  // now, we have to unsure that the coefficient of each fild of the rank is set
  std::for_each(model.rank_fields.begin(), model.rank_fields.end(), [&model](rank_field_model& field) {
    if (field.name.empty()) {
      error("Empty field name in the rank definition for state \"", model.name, "\"");
      throw std::runtime_error("state model: empty rank field");
    }
    if (field.coefficient.empty()) {
      warning("Automatically set \"1\" to coefficient of field \"", field.name, "\" in state \"", model.name,
              "\"");
      field.coefficient = "1";
    }
  });

  // next, we need to validate all the information on the constraint models of a state
  std::for_each(model.constraints.begin(), model.constraints.end(), [&model](constraint_model& constraint) {
    if (constraint.name.empty()) {
      error("Empty field name in a constraint for state \"", model.name, "\"");
      throw std::runtime_error("state model: empty constraint field");
    }
    if (constraint.value.empty()) {
      error("The constraint on \"", constraint.name, "\" in state \"", model.name, "\" has no goal value");
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
                    r.kind = subject_kind::KNOB;
                  } else if (is_a_metric(r.name)) {
                    r.kind = subject_kind::METRIC;
                  } else {
                    error("The rank field \"", r.name, "\" is not metric nor a knob");
                    throw std::runtime_error("state model: unknown rank field");
                  }
                });
  std::for_each(model.constraints.begin(), model.constraints.end(),
                [&is_a_metric, &is_a_knob](constraint_model& c) {
                  if (is_a_knob(c.name)) {
                    c.kind = subject_kind::KNOB;
                  } else if (is_a_metric(c.name)) {
                    c.kind = subject_kind::METRIC;
                  } else {
                    error("The constraint on field \"", c.name, "\" is not on a metric nor a nob");
                    throw std::runtime_error("state model: unknown constraint field");
                  }
                });
}

}  // namespace heel
}  // namespace margot