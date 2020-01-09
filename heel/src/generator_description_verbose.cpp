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

#include <sstream>

#include <heel/generator_description_verbose.hpp>
#include <heel/generator_utils.hpp>

namespace margot {
namespace heel {

std::stringstream description_verbose(const monitor_model& model) {
  // declare the stream that will hold the monitor synthetic description
  std::stringstream d;

  // provide the most boring information regarding its class
  d << "Monitor \"" << model.name << "\"" << std::endl;
  d << "\tType: " << model.type << std::endl;

  // provide the information about the logged measures
  if (!model.requested_statistics.empty()) {
    d << "\tLogging: [";
    d << join(model.requested_statistics.cbegin(), model.requested_statistics.cend(), ", ",
              [](const std::string& property) { return property; });
    d << "]" << std::endl;
  }

  // provide the information about the constructor
  d << "\tConstructor (";
  d << join(model.initialization_parameters.cbegin(), model.initialization_parameters.cend(), ", ",
            [](const parameter& parameter) { return parameter.content; });
  d << ")" << std::endl;

  // provide the information about the start method (if any)
  if (!model.start_parameters.empty()) {
    d << "\tStart(";
    d << join(model.start_parameters.cbegin(), model.start_parameters.cend(), ", ",
              [](const parameter& parameter) { return parameter.content; });
    d << ")" << std::endl;
  }

  // provide the information about the start method (if any)
  if (!model.stop_parameters.empty()) {
    d << "\tStop(";
    d << join(model.stop_parameters.cbegin(), model.stop_parameters.cend(), ", ",
              [](const parameter& parameter) { return parameter.content; });
    d << ")" << std::endl;
  }

  return d;
}

std::stringstream description_verbose(const knob_model& model) {
  std::stringstream d;
  d << "Knob \"" << model.name << "\"" << std::endl;
  d << "\tType: " << model.type << std::endl;
  if (!model.values.empty()) {
    d << "\tValues: [";
    d << join(model.values.cbegin(), model.values.cend(), "; ",
              [](const std::string& value) { return value; });
    d << "]" << std::endl;
  }
  return d;
}

std::stringstream description_verbose(const metric_model& model) {
  std::stringstream d;
  d << "Metric \"" << model.name << "\"" << std::endl;
  d << "\tType: " << model.type << std::endl;
  d << "\tIs a distribution: " << (model.distribution ? std::string("true") : std::string("false"))
    << std::endl;
  if (!model.monitor_name.empty()) {
    d << "\tObserved by monitor \"" << model.monitor_name << "\"";
  }
  if (model.inertia > 0) {
    d << " [REACTIVE INERTIA " << model.inertia << ']';
  }
  d << std::endl;
  if (!model.prediction_plugin.empty()) {
    d << "\tPrediction plugin: " << model.prediction_plugin << std::endl;
    d << "\t           params: [";
    d << join(model.prediction_parameters.cbegin(), model.prediction_parameters.cend(), ", ",
              [](const pair_property& p) { return p.key + "=" + p.value; })
      << "]" << std::endl;
  }
  return d;
}

std::stringstream description_verbose(const features_model& model) {
  std::stringstream d;
  if (!model.empty()) {
    d << "Using features" << std::endl;
    d << "\tDistance type: " << to_str(model.distance_type) << std::endl;
    d << "\tFields: \""
      << join(model.fields.cbegin(), model.fields.cend(), "\", \"",
              [](const feature_model& model) { return model.name + "::" + model.type; })
      << "\"" << std::endl;
  }
  return d;
}

std::stringstream description_verbose(const agora_model& model) {
  std::stringstream d;
  if (!model.empty()) {
    d << "Relying on \"agora\" to obtain the application knowledge" << std::endl;
    d << "\tConnection url: \"" << model.url << "\"" << std::endl;
    d << "\t           username: \"" << model.username << "\"" << std::endl;
    d << "\t           password: \"" << model.password << "\"" << std::endl;
    d << "\t           qos: \"" << model.qos << "\"" << std::endl;
    d << "\t           broker_ca: \"" << model.broker_ca << "\"" << std::endl;
    d << "\t           client_cert: \"" << model.client_cert << "\"" << std::endl;
    d << "\t           client_key: \"" << model.client_key << "\"" << std::endl;
    d << "\tClustering plugin:\"" << model.clustering_plugin << "\"" << std::endl;
    d << "\t           params: [";
    d << join(model.clustering_parameters.cbegin(), model.clustering_parameters.cend(), ", ",
              [](const pair_property& p) { return p.key + "=" + p.value; })
      << "]" << std::endl;
    d << "\tDoe        plugin:\"" << model.doe_plugin << "\"" << std::endl;
    d << "\t           params: [";
    d << join(model.doe_parameters.cbegin(), model.doe_parameters.cend(), ", ",
              [](const pair_property& p) { return p.key + "=" + p.value; })
      << "]" << std::endl;
  }
  return d;
}

std::stringstream description_verbose(const state_model& model) {
  std::stringstream d;
  d << "Extra-functional requirements \"" << model.name << "\"" << std::endl;
  d << "\t";
  if (model.direction == rank_direction::MINIMIZE) {
    d << "minimize ";
  } else {
    d << "maximize ";
  }
  if (model.combination == rank_type::SIMPLE) {
    d << "(";
  } else if (model.combination == rank_type::GEOMETRIC) {
    d << " gemetric_avg(";
  } else if (model.combination == rank_type::LINEAR) {
    d << " linear_avg(";
  }
  d << join(model.rank_fields.cbegin(), model.rank_fields.cend(), ", ",
            [](const rank_field_model& p) { return p.name + "::" + p.coefficient; })
    << ")" << std::endl;
  if (!model.constraints.empty()) {
    d << "\t subject to:" << std::endl;
    for (const auto& c : model.constraints) {
      d << "\t  " << c.name << " ";
      if (c.cfun == goal_comparison::LESS_OR_EQUAL) {
        d << "<=";
      } else if (c.cfun == goal_comparison::GREATER_OR_EQUAL) {
        d << ">=";
      } else if (c.cfun == goal_comparison::LESS) {
        d << "<";
      } else if (c.cfun == goal_comparison::GREATER) {
        d << ">";
      } else {
        d << "UNK";
      }
      d << " " << c.value;
      if (!c.confidence.empty()) {
        d << " with confidence " << c.confidence;
      }
      d << std::endl;
    }
  }

  return d;
}

}  // namespace heel
}  // namespace margot