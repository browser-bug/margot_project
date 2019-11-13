#include <algorithm>
#include <cstdint>
#include <string>

#include <heel/cpp_interface_conversion.hpp>
#include <heel/generator_cpp_managers_hdr.hpp>
#include <heel/generator_utils.hpp>
#include <heel/logger.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/model_monitor.hpp>
#include <heel/model_state.hpp>

margot::heel::cpp_source_content margot::heel::managers_hdr_content(application_model& app) {
  margot::heel::cpp_source_content c;

  // append by default the cstdint and the application geometry header
  c.required_headers.emplace_back("cstdint");
  c.required_headers.emplace_back("margot/application_geometry.hpp");

  // generate the content for each block managed by margot, within its namespace
  c.content << std::endl << "namespace margot {" << std::endl;
  std::for_each(app.blocks.begin(), app.blocks.end(), [&c](block_model& block) {
    c.content << "class " << block.name << " {" << std::endl;
    c.content << "public:" << std::endl;

    // define the struct that contains the monitors (and append the related headers)
    c.content << "\tstruct monitors_type {" << std::endl;
    std::for_each(block.monitors.begin(), block.monitors.end(), [&c](monitor_model& monitor) {
      const auto& spec = margot::heel::get_monitor_cpp_spec(monitor);
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
                      if (constraint.kind == margot::heel::constraint_subject_kind::METRIC) {
                        c.content << block.metrics_segment_type;
                      } else if (constraint.kind == margot::heel::constraint_subject_kind::KNOB) {
                        c.content << block.knobs_segment_type;
                      } else {
                        margot::heel::error("Unknown type of goal, this looks like an internal error");
                        throw std::runtime_error("manager generators: unknown constraint type");
                      }
                      c.content << "> " << state.name << "_constraint_" << counter++ << ";" << std::endl;
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

    // declare the content of the struct
    c.content << "\tmargot::" << block.name << "_utils::manager_type manager;" << std::endl;
    c.content << "\tmonitors_type monitors;" << std::endl;
    c.content << "\tgoals_type goals;" << std::endl;
    c.content << "\tknobs_type knobs;" << std::endl << std::endl;

    // declare the constructor e destructor of the struct that are deleted
    c.content << "\t" << block.name << "(const " << block.name << "&) = delete;" << std::endl;
    c.content << "\t" << block.name << "(" << block.name << "&&) = delete;" << std::endl << std::endl;

    // declare the function that retrieves the only object that should exist
    c.content << "\tinline static " << block.name << "& interface(void) {" << std::endl;
    c.content << "\t\tstatic " << block.name << " instance;" << std::endl;
    c.content << "\t\treturn instance;" << std::endl;
    c.content << "\t}" << std::endl;

    // now we define the actual destructor and constructor as private, to implement the singleton pattern
    c.content << "private:" << std::endl;
    c.content << "\t" << block.name << "(void);" << std::endl;
    c.content << "\t~" << block.name << "(void);" << std::endl << std::endl;

    c.content << "}; // end class " << block.name << std::endl << std::endl;
  });
  c.content << "} // namespace margot" << std::endl << std::endl;

  return c;
}