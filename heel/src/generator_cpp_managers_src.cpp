#include <algorithm>
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

margot::heel::cpp_source_content margot::heel::managers_cpp_content(margot::heel::application_model& app) {
  margot::heel::cpp_source_content c;

  // append by default the cstdint and the application geometry header
  c.required_headers.emplace_back("cstdint");
  c.required_headers.emplace_back("margot/application_geometry.hpp");
  c.required_headers.emplace_back("margot/managers_definition.hpp");

  // generate the content of the required functions for each block
  std::for_each(app.blocks.begin(), app.blocks.end(), [&c](const margot::heel::block_model& block) {
    // -------- generate the constructor of the class
    c.content << "margot::" << block.name << "::" << block.name << "(void):";
    // the monitors and goals are initialized with the initializer list. However, we initialize here only the
    // monitors that have all the initialization parameters "immediate".
    c.content << "monitors({"
              << margot::heel::join(
                     block.monitors.begin(), block.monitors.end(), ",",
                     [](const margot::heel::monitor_model& monitor) {
                       if (std::none_of(monitor.initialization_parameters.begin(),
                                        monitor.initialization_parameters.end(),
                                        [](const margot::heel::parameter& p) {
                                          return p.type == margot::heel::parameter_types::VARIABLE;
                                        })) {
                         return "{" +
                                margot::heel::join(
                                    monitor.initialization_parameters.begin(),
                                    monitor.initialization_parameters.end(), ",",
                                    [](const margot::heel::parameter& p) { return p.content; }) +
                                "}";
                       } else {
                         return std::string("{}");
                       }
                     });
    c.content << "}),";
    std::vector<std::string> goal_values;
    std::for_each(block.states.begin(), block.states.end(),
                  [&goal_values](const margot::heel::state_model& state) {
                    std::for_each(state.constraints.begin(), state.constraints.end(),
                                  [&goal_values](const margot::heel::constraint_model& constraint) {
                                    goal_values.emplace_back(constraint.value);
                                  });
                  });
    c.content << "goals({"
              << margot::heel::join(goal_values.begin(), goal_values.end(), ",",
                                    [](const std::string value) { return "{" + value + "}"; })
              << "}) ";

    // now it is time to define the body of the constructor
    c.content << "{" << std::endl;

    // before defining the extra-functional requirements, we should add the application knowledge
    c.required_headers.emplace_back("margot/application_knowledge.hpp");
    c.content << "\tmargot::add_application_knowledge(manager);" << std::endl;

    // maybe we need to add an information provider for a metric
    std::for_each(block.metrics.begin(), block.metrics.end(), [&c, &block](const metric_model& metric) {
      if (metric.inertia > 0) {
        c.content << "\tmanager.add_runtime_knowledge<margot::OperatingPointSegments::METRICS,"
                  << generate_field_getter(margot::heel::subject_kind::METRIC, metric.name, block.name) << ','
                  << metric.inertia << ">(monitors." << metric.monitor_name << ");" << std::endl;
      }
    });

    // it's time to create and define all the states of agora for this block
    std::for_each(block.states.begin(), block.states.end(), [&c, &block](const state_model& state) {
      margot::heel::append(c, margot::heel::generate_cpp_content(state, block.name), "\t");
    });

    c.content << "}" << std::endl << std::endl;
  });

  return c;
}