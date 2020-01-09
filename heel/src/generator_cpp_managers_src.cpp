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
#include <cctype>
#include <cstdint>

#include <heel/cpp_enum_conversion.hpp>
#include <heel/cpp_state_emitter.hpp>
#include <heel/cpp_utils.hpp>
#include <heel/generator_cpp_managers_src.hpp>
#include <heel/generator_utils.hpp>
#include <heel/logger.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/model_monitor.hpp>
#include <heel/model_state.hpp>

namespace margot {
namespace heel {

cpp_source_content managers_cpp_content(application_model& app, const std::string& app_description) {
  cpp_source_content c;

  // append by default the cstdint and the application geometry header
  c.required_headers.emplace_back("cstdint");
  c.required_headers.emplace_back("margot/application_geometry.hpp");
  c.required_headers.emplace_back("margot/managers_definition.hpp");

  // generate the content of the required functions for each block
  std::for_each(app.blocks.begin(), app.blocks.end(), [&](const block_model& block) {
    // -------- generate the constructor of the class
    c.content << "margot::" << block.name << "::data::data(void):";
    // the monitors and goals are initialized with the initializer list. However, we initialize here only the
    // monitors that have all the initialization parameters "immediate".
    c.content << "monitors({"
              << join(block.monitors.begin(), block.monitors.end(), ",", [](const monitor_model& monitor) {
                   if (std::none_of(monitor.initialization_parameters.begin(),
                                    monitor.initialization_parameters.end(),
                                    [](const parameter& p) { return p.type == parameter_types::VARIABLE; })) {
                     return "{" +
                            join(monitor.initialization_parameters.begin(),
                                 monitor.initialization_parameters.end(), ",",
                                 [](const parameter& p) { return p.content; }) +
                            "}";
                   } else {
                     return std::string("{}");
                   }
                 });
    c.content << "}),";
    std::vector<std::string> goal_values;
    std::for_each(block.states.begin(), block.states.end(), [&goal_values](const state_model& state) {
      std::for_each(
          state.constraints.begin(), state.constraints.end(),
          [&goal_values](const constraint_model& constraint) { goal_values.emplace_back(constraint.value); });
    });
    c.content << "goals({" << join(goal_values.begin(), goal_values.end(), ",", [](const std::string value) {
      return "{" + value + "}";
    }) << "}) ";

    // now it is time to define the body of the constructor
    c.content << "{" << std::endl;

    // check if we need to initialize the state or if it just about profiling
    if (!block.knobs.empty()) {
      // before defining the extra-functional requirements, we should add the application knowledge
      c.required_headers.emplace_back("margot/application_knowledge.hpp");
      c.content << "\tmargot::add_application_knowledge(manager);" << std::endl;

      // maybe we need to add an information provider for a metric
      std::for_each(block.metrics.begin(), block.metrics.end(), [&c, &block](const metric_model& metric) {
        if (metric.inertia > 0) {
          c.content << "\tmanager.add_runtime_knowledge<margot::OperatingPointSegments::METRICS,"
                    << generate_field_getter(subject_kind::METRIC, metric.name, block.name) << ','
                    << metric.inertia << ">(monitors." << metric.monitor_name << ");" << std::endl;
        }
      });

      // it's time to create and define all the states of agora for this block
      std::for_each(block.states.begin(), block.states.end(), [&c, &block](const state_model& state) {
        append(c, generate_cpp_content(state, block.name), "\t");
      });

      // if agora is enabled, we need to start the local thread that communicates the remote handler
      if (!block.agora.empty()) {
        c.required_headers.emplace_back("margot/application_geometry.hpp");
        // before generating the command, we need to pre-process the application description
        const std::string escaped =
            join(app_description.begin(), app_description.end(), "", [](const std::string::value_type c) {
              if (c == '"') return std::string("\\\"");
              if (std::isspace(c)) return std::string(" ");
              return std::string(1, c);
            });
        std::string stripped;
        std::unique_copy(escaped.begin(), escaped.end(), std::back_insert_iterator<std::string>(stripped),
                         [](const char a, const char b) { return std::isspace(a) && std::isspace(b); });
        c.content << "\tmanager.start_support_thread<margot::" << block.name << "::operating_point_parser>(\""
                  << app.name << '/' << app.version << '/' << block.name << "\",\"" << block.agora.url
                  << "\",\"" << block.agora.username << "\",\"" << block.agora.password << "\","
                  << block.agora.qos << ",\"" << stripped << "\",\"" << block.agora.broker_ca << "\",\""
                  << block.agora.client_cert << "\",\"" << block.agora.client_key << "\");" << std::endl;
      }
    }
    c.content << "}" << std::endl << std::endl;
  });

  return c;
}

}  // namespace heel
}  // namespace margot