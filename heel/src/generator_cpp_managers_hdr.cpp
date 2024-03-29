/* mARGOt HEEL library
 * Copyright (C) 2018 Davide Gadioli
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
#include <cstdint>
#include <string>

#include <heel/cpp_enum_conversion.hpp>
#include <heel/cpp_parser_gen.hpp>
#include <heel/cpp_utils.hpp>
#include <heel/generator_cpp_managers_hdr.hpp>
#include <heel/generator_description_verbose.hpp>
#include <heel/generator_utils.hpp>
#include <heel/logger.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/model_knob.hpp>
#include <heel/model_metric.hpp>
#include <heel/model_monitor.hpp>
#include <heel/model_state.hpp>

namespace margot {
namespace heel {

cpp_source_content managers_hpp_content(application_model& app) {
  cpp_source_content c;

  // append by default the cstdint and the application geometry header
  c.required_headers.emplace_back("cstdint");
  c.required_headers.emplace_back("fstream");
  c.required_headers.emplace_back("margot/application_geometry.hpp");
  c.required_headers.emplace_back("mutex");

  // generate the content for each block managed by margot, within its namespace
  c.content << std::endl << "namespace margot {" << std::endl << std::endl;
  std::for_each(app.blocks.begin(), app.blocks.end(), [&c](block_model& block) {
    // print the description of the application model as comment before generating the source code of the
    // class. In this way, the user can have an idea to what has been generated in the code
    c.content << "namespace " << block.name << " {" << std::endl;
    c.content << "// ------------------===={ Block \"" << block.name << "\" model }====------------------"
              << std::endl;
    const auto print_desc = [&c](std::stringstream stream) {
      for (std::string line; std::getline(stream, line); /* internally handled */) {
        c.content << "//    " << line << std::endl;
      }
      c.content << "//" << std::endl;
    };
    std::for_each(block.monitors.begin(), block.monitors.end(),
                  [&print_desc](const monitor_model& monitor) { print_desc(description_verbose(monitor)); });
    std::for_each(block.metrics.begin(), block.metrics.end(),
                  [&print_desc](const metric_model& metric) { print_desc(description_verbose(metric)); });
    std::for_each(block.knobs.begin(), block.knobs.end(),
                  [&print_desc](const knob_model& knob) { print_desc(description_verbose(knob)); });
    if (!block.features.fields.empty()) {
      print_desc(description_verbose(block.features));
    }
    if (!block.agora.empty()) {
      print_desc(description_verbose(block.agora));
    }
    std::for_each(block.states.begin(), block.states.end(),
                  [&print_desc](const state_model& state) { print_desc(description_verbose(state)); });
    c.content << "// ------------------=============" << std::string(block.name.size(), '=')
              << "=============------------------" << std::endl;

    // now we can generate the code for the actual class that handles the application
    c.content << "class data {" << std::endl;
    c.content << "public:" << std::endl;

    // define the struct that contains the monitors (and append the related headers)
    c.content << "\tstruct monitors_type {" << std::endl;
    std::for_each(block.monitors.begin(), block.monitors.end(), [&c](monitor_model& monitor) {
      const auto& spec = get_monitor_cpp_spec(monitor);
      c.content << "\t\t" << spec.class_name << ' ' << monitor.name << ';' << std::endl;
      c.required_headers.emplace_back(spec.header_name);
    });
    c.content << "\t};" << std::endl << std::endl;

    // define the struct that contains the goal of each constraint in each state. The tricky part is to figure
    // out the type of the goal
    c.content << "\tstruct goals_type {" << std::endl;
    std::for_each(block.states.begin(), block.states.end(), [&c, &block](const state_model& state) {
      std::size_t counter = 0;
      std::for_each(state.constraints.begin(), state.constraints.end(),
                    [&c, &block, &state, &counter](const constraint_model& constraint) {
                      c.required_headers.emplace_back("margot/goal.hpp");
                      c.content << "\t\tmargot::Goal<";
                      switch (constraint.kind) {
                        case subject_kind::METRIC:
                          c.content << block.metrics_segment_type;
                          break;
                        case subject_kind::KNOB:
                          c.content << block.knobs_segment_type;
                          break;
                        default:
                          error("Unknown constraint kind, this looks like an internal error");
                          throw std::runtime_error("manager generators: unknown constraint kind");
                      }
                      c.content << ',' << cpp_enum::get(constraint.cfun) << "> "
                                << generate_goal_identifier(state.name, counter) << ';' << std::endl;
                    });
    });
    c.content << "\t};" << std::endl << std::endl;

    // define the struct that contains the knobs of the block
    c.content << "\tstruct knobs_type {" << std::endl;
    std::for_each(block.knobs.begin(), block.knobs.end(), [&c](const knob_model& knob) {
      c.content << "\t\t";
      if (knob.type.rfind("string") == 0) {
        c.required_headers.emplace_back("string");
        c.content << "std::";
      }
      c.content << knob.type << " " << knob.name << ";" << std::endl;
    });
    c.content << "\t};" << std::endl << std::endl;

    // define the struct that containst the features of the block
    c.content << "\tstruct features_type {" << std::endl;
    std::for_each(block.features.fields.begin(), block.features.fields.end(),
                  [&c](const feature_model& feature) {
                    c.content << "\t\t" << feature.type << " " << feature.name << ";" << std::endl;
                  });
    c.content << "\t};" << std::endl << std::endl;

    // declare the content of the struct
    if (!block.knobs.empty()) {
      c.content << "\tmargot::" << block.name << "::manager_type manager;" << std::endl;
    }
    c.content << "\tmonitors_type monitors;" << std::endl;
    c.content << "\tgoals_type goals;" << std::endl;
    c.content << "\tknobs_type knobs;" << std::endl;
    c.content << "\tfeatures_type features;" << std::endl;
    c.content << "\tstd::ofstream log_file;" << std::endl;
    c.content << "\tstd::mutex context_mux;" << std::endl << std::endl;

    // declare the constructor e destructor of the struct that are deleted
    c.content << "\tdata(const data&) = delete;" << std::endl;
    c.content << "\tdata(data&&) = delete;" << std::endl << std::endl;

    // declare the function that retrieves the only object that should exist
    c.content << "\tinline static data& get_instance(void) {" << std::endl;
    c.content << "\t\tstatic data instance;" << std::endl;
    c.content << "\t\treturn instance;" << std::endl;
    c.content << "\t}" << std::endl;

    // now we define the actual destructor and constructor as private, to implement the singleton pattern
    c.content << "private:" << std::endl;
    c.content << "\tdata(void);" << std::endl << std::endl;

    c.content << "}; // end class " << block.name << std::endl;
    c.content << "} // namespace " << block.name << std::endl << std::endl;
  });
  c.content << "} // namespace margot" << std::endl << std::endl;

  return c;
}

}  // namespace heel
}  // namespace margot