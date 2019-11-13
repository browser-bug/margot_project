#include <algorithm>

#include <heel/generator_cpp_managers_src.hpp>
#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/model_monitor.hpp>

margot::heel::cpp_source_content margot::heel::managers_cpp_content(margot::heel::application_model& app) {
  margot::heel::cpp_source_content c;

  // append by default the cstdint and the application geometry header
  c.required_headers.emplace_back("cstdint");
  c.required_headers.emplace_back("margot/application_geometry.hpp");
  c.required_headers.emplace_back("margot/managers_definition.hpp");

  // generate the content of the required functions for each block
  std::for_each(app.blocks.begin(), app.blocks.end(), [&c](const margot::heel::block_model& block) {
    // -------- generate the constructor of the class
    c.content << "margot::" << block.name << "::" << block.name << "(void)" << std::endl;
    c.content << " :monitors({";
    std::for_each(
        block.monitors.begin(), block.monitors.end(), [&c](const margot::heel::monitor_model& monitor) {
          // immidiately initialize the monitors that have an "immediate" initialization
          if (std::none_of(monitor.initialization_parameters.begin(), monitor.initialization_parameters.end(),
                           [](const margot::heel::parameter& p) {
                             return p.type == margot::heel::parameter_types::VARIABLE;
                           })) {
            c.content << "{"
                      << margot::heel::join(monitor.initialization_parameters.begin(),
                                            monitor.initialization_parameters.end(), ",",
                                            [&c](const margot::heel::parameter& p) { return p.content; });
            c.content << "}";
          } else {
            c.content << "{}";
          }
          c.content << ",";
        });
    c.content << "}),goals({";
    std::for_each(block.states.begin(), block.states.end(), [&c](const margot::heel::state_model& state) {
      std::for_each(state.constraints.begin(), state.constraints.end(),
                    [&c](const margot::heel::constraint_model& constraint) {
                      c.content << '{' << constraint.value << "},";
                    });
    });
    c.content << "}) {" << std::endl;
    c.content << "}" << std::endl << std::endl;
  });

  return c;
}