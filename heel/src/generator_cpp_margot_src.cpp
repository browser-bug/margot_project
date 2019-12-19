#include <algorithm>

#include <heel/cpp_init_gen.hpp>
#include <heel/cpp_parser_gen.hpp>
#include <heel/cpp_push_monitor_gen.hpp>
#include <heel/cpp_start_monitor_gen.hpp>
#include <heel/cpp_stop_monitor_gen.hpp>
#include <heel/cpp_update_gen.hpp>
#include <heel/cpp_utils.hpp>
#include <heel/generator_cpp_margot_src.hpp>
#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/model_knob.hpp>
#include <heel/model_monitor.hpp>
#include <heel/model_parameter.hpp>

margot::heel::cpp_source_content global_init_content(const margot::heel::application_model& app) {
  margot::heel::cpp_source_content c;
  // the goal of this function is twofold. On one hand, it has to initialize all the monitors that requires an
  // input variable in the constructor. On the other hand, it enforces the initializations of all the
  // managers.
  std::for_each(app.blocks.begin(), app.blocks.end(), [&c](const margot::heel::block_model& block) {
    c.required_headers.emplace_back("mutex");
    bool need_to_regenerate_information_providers = false;
    c.content << "{" << std::endl;  // we use the block scope to avoid name clashing and to lock it
    c.content << "\tauto& c = margot::" << block.name << "::context();" << std::endl;
    c.content << "\tconst std::lock_guard<std::mutex> lock(c.context_mux);" << std::endl;

    // initialize the monitors that requires a variable in the constructor
    std::for_each(
        block.monitors.begin(), block.monitors.end(), [&](const margot::heel::monitor_model& monitor) {
          const bool to_overwrite =
              std::any_of(monitor.initialization_parameters.begin(), monitor.initialization_parameters.end(),
                          [](const margot::heel::parameter& p) {
                            return p.type == margot::heel::parameter_types::VARIABLE;
                          });
          if (to_overwrite) {
            // get the monitor spec and assign the new monitor
            const auto& spec = margot::heel::get_monitor_cpp_spec(monitor.type);
            need_to_regenerate_information_providers =
                need_to_regenerate_information_providers ||
                std::any_of(block.metrics.begin(), block.metrics.end(),
                            [&monitor](const margot::heel::metric_model& metric) {
                              return metric.inertia > 0 && metric.monitor_name.compare(monitor.name) == 0;
                            });
            c.required_headers.emplace_back(spec.header_name);
            c.content << "\tc.monitors." << monitor.name << " = " << spec.class_name << "("
                      << margot::heel::join(monitor.initialization_parameters.begin(),
                                            monitor.initialization_parameters.end(), ", ",
                                            [](const margot::heel::parameter& p) { return p.content; })
                      << ");" << std::endl;
          }
        });

    // check if we need to reset the information providers
    if (need_to_regenerate_information_providers) {
      c.content << "\tc.manager.remove_all_runtime_knowledge();" << std::endl;
      std::for_each(
          block.metrics.begin(), block.metrics.end(), [&c, &block](const margot::heel::metric_model& metric) {
            if (metric.inertia > 0) {
              c.content << "\tc.manager.add_runtime_knowledge<margot::OperatingPointSegments::METRICS,"
                        << generate_field_getter(margot::heel::subject_kind::METRIC, metric.name, block.name)
                        << ',' << metric.inertia << ">(c.monitors." << metric.monitor_name << ");"
                        << std::endl;
            }
          });
    }
    c.content << "}" << std::endl;
  });
  return c;
}

margot::heel::cpp_source_content update_content(const margot::heel::block_model& block) {
  margot::heel::cpp_source_content c;
  c.required_headers.emplace_back("mutex");
  if (!block.knobs.empty()) {
    c.content << "auto& c = margot::" << block.name << "::context();" << std::endl;
    c.content << "const std::lock_guard<std::mutex> lock(c.context_mux);" << std::endl;
    c.content << "bool conf_changed = false;" << std::endl;
    if (!block.features.fields.empty()) {
      std::for_each(block.features.fields.begin(), block.features.fields.end(),
                    [&c](const margot::heel::feature_model& f) {
                      c.content << "c.features." << f.name << " = " << f.name << ";" << std::endl;
                    });
      c.content << "c.manager.select_feature_cluster({{";
      c.content << margot::heel::join(block.features.fields.begin(), block.features.fields.end(), ", ",
                                      [](const margot::heel::feature_model& f) { return f.name; });
      c.content << "}});" << std::endl;
    }
    c.content << "if (!c.manager.is_application_knowledge_empty()) {" << std::endl;
    c.content << "\tc.manager.find_best_configuration();" << std::endl;
    c.content << "\tconst auto new_conf = c.manager.get_best_configuration(&conf_changed);" << std::endl;
    c.content << "\tif (conf_changed) {" << std::endl;
    std::for_each(block.knobs.begin(), block.knobs.end(), [&c, &block](const margot::heel::knob_model& knob) {
      c.content << "\t\t" << knob.name << " = ";
      const std::string getter =
          "new_conf.get_mean<" +
          generate_field_getter(margot::heel::subject_kind::KNOB, knob.name, block.name) + ">()";
      const std::string lhs_assignment =
          knob.type.compare("string") != 0
              ? getter
              : "margot::" + block.name + "::val_to_knob_" + knob.name + "(" + getter + ")";
      c.content << lhs_assignment << ";" << std::endl;
    });
    c.content << "\t\tc.manager.configuration_applied();" << std::endl;
    c.content << "\t}" << std::endl;
    c.content << "}" << std::endl;
    std::for_each(block.knobs.begin(), block.knobs.end(), [&c](const margot::heel::knob_model& knob) {
      c.content << "c.knobs." << knob.name << " = " << knob.name << ';' << std::endl;
    });
    c.content << "return conf_changed;" << std::endl;
  } else {
    c.content << "return false;" << std::endl;
  }

  return c;
}

margot::heel::cpp_source_content start_monitor_content(const margot::heel::block_model& block) {
  margot::heel::cpp_source_content c;
  c.content << "auto& c = margot::" << block.name << "::context();" << std::endl;
  c.content << "const std::lock_guard<std::mutex> lock(c.context_mux);" << std::endl;
  std::for_each(
      block.monitors.begin(), block.monitors.end(), [&c](const margot::heel::monitor_model& monitor) {
        const auto& spec = margot::heel::get_monitor_cpp_spec(monitor.type);
        if (!spec.start_method_name.empty()) {
          c.content << "c.monitors." << monitor.name << "." << spec.start_method_name << "(";
          c.content << margot::heel::join(monitor.start_parameters.begin(), monitor.start_parameters.end(),
                                          ", ", [](const margot::heel::parameter& p) { return p.content; });
          c.content << ");" << std::endl;
        }
      });
  return c;
}

margot::heel::cpp_source_content stop_monitor_content(const margot::heel::block_model& block) {
  margot::heel::cpp_source_content c;
  c.required_headers.emplace_back("mutex");
  c.content << "auto& c = margot::" << block.name << "::context();" << std::endl;
  c.content << "const std::lock_guard<std::mutex> lock(c.context_mux);" << std::endl;
  std::for_each(
      block.monitors.begin(), block.monitors.end(), [&c](const margot::heel::monitor_model& monitor) {
        const auto& spec = margot::heel::get_monitor_cpp_spec(monitor.type);
        if (!spec.stop_method_name.empty()) {
          if (spec.stop_method_name.compare("push") != 0) {
            c.content << "c.monitors." << monitor.name << "." << spec.stop_method_name << "(";
            c.content << margot::heel::join(monitor.stop_parameters.begin(), monitor.stop_parameters.end(),
                                            ", ", [](const margot::heel::parameter& p) { return p.content; });
            c.content << ");" << std::endl;
          }
        }
      });
  return c;
}

margot::heel::cpp_source_content push_monitor_content(const margot::heel::block_model& block) {
  margot::heel::cpp_source_content c;
  if (!block.knobs.empty()) {
    c.required_headers.emplace_back("mutex");
    c.content << "auto& c = margot::" << block.name << "::context();" << std::endl;
    c.content << "const std::lock_guard<std::mutex> lock(c.context_mux);" << std::endl;
    std::for_each(
        block.monitors.begin(), block.monitors.end(), [&c](const margot::heel::monitor_model& monitor) {
          const auto& spec = margot::heel::get_monitor_cpp_spec(monitor.type);
          if (!spec.stop_method_name.empty()) {
            if (spec.stop_method_name.compare("push") == 0) {
              c.content << "c.monitors." << monitor.name << ".push(";
              c.content << margot::heel::join(monitor.stop_parameters.begin(), monitor.stop_parameters.end(),
                                              ", ",
                                              [](const margot::heel::parameter& p) { return p.content; });
              c.content << ");" << std::endl;
            }
          }
        });

    // check if we need to generate the call to the function that sends the information to agora
    if (!block.agora.empty()) {
      c.required_headers.emplace_back("margot/application_geometry.hpp");
      c.content << "margot::" << block.name << "::operating_point_parser p;" << std::endl;
      c.content << "c.manager.send_observation(p("
                << margot::heel::cpp_parser_gen::usage(block.features.fields, block.knobs, block.metrics)
                << "));" << std::endl;
    }
  }
  return c;
}

margot::heel::cpp_source_content log_content(const margot::heel::block_model& block) {
  margot::heel::cpp_source_content c;
  c.content << "// TBD" << std::endl;
  return c;
}

margot::heel::cpp_source_content margot::heel::margot_cpp_content(
    const margot::heel::application_model& app) {
  margot::heel::cpp_source_content c;

  // generate the preamble of the header
  c.required_headers.emplace_back("string");
  c.required_headers.emplace_back("cstdint");
  c.required_headers.emplace_back("margot/managers_definition.hpp");
  c.required_headers.emplace_back("margot/application_geometry.hpp");
  c.required_headers.emplace_back("margot/margot.hpp");
  c.content << "namespace margot {" << std::endl << std::endl;

  // generate the content of the global init function
  c.content << "void init(" << margot::heel::cpp_init_gen::signature(app) << ") {" << std::endl;
  margot::heel::append(c, global_init_content(app), "\t");
  c.content << "}" << std::endl;

  // loop over each block to generate the block-specific functions
  std::for_each(app.blocks.begin(), app.blocks.end(), [&c](const block_model& block) {
    c.content << std::endl << "namespace " << block.name << " {" << std::endl;
    c.content << "bool update(" << margot::heel::cpp_update_gen::signature(block) << ") {" << std::endl;
    margot::heel::append(c, update_content(block), "\t");
    c.content << "}" << std::endl;
    c.content << "void start_monitors(" << margot::heel::cpp_start_monitor_gen::signature(block) << ") {"
              << std::endl;
    margot::heel::append(c, start_monitor_content(block), "\t");
    c.content << "}" << std::endl;
    c.content << "void stop_monitors(" << margot::heel::cpp_stop_monitor_gen::signature(block) << ") {"
              << std::endl;
    margot::heel::append(c, stop_monitor_content(block), "\t");
    c.content << "}" << std::endl;
    c.content << "void push_custom_monitor_values(" << margot::heel::cpp_push_monitor_gen::signature(block)
              << ") {" << std::endl;
    margot::heel::append(c, push_monitor_content(block), "\t");
    c.content << "}" << std ::endl;
    c.content << "void log(void) {" << std::endl;
    margot::heel::append(c, log_content(block), "\t");
    c.content << "}" << std::endl;
    c.content << "} // namespace " << block.name << std::endl;
  });

  // generate the trailer of the header
  c.content << std::endl << "} // namespace margot" << std::endl;

  return c;
}