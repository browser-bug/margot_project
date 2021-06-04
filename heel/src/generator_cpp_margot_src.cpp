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

    // we need to emit the code that open the log file and write the header
    c.content << "\t#ifdef MARGOT_ENABLE_FILE_LOG" << std::endl;
    c.content << "\tc.log_file.open(" << margot::heel::generate_log_file_name_identifier() << " + \"margot."
              << block.name << ".log\");" << std::endl;
    c.content << "\tc.log_file << \"timestamp\" ";
    for (const auto& monitor : block.monitors) {
      for (const auto& output_statistic : monitor.requested_statistics) {
        c.content << "<< \",monitor_" << monitor.name << "_" << output_statistic << "\" ";
      }
    }
    if (!block.knobs.empty()) {
      for (const auto& metric : block.metrics) {
        c.content << "<< \",metric_" << metric.name << "\" ";
      }
      for (const auto& feature : block.features.fields) {
        c.content << "<< \",feature_" << feature.name << "\" ";
      }
      for (const auto& knob : block.knobs) {
        c.content << "<< \",knob_" << knob.name << "\" ";
      }
      for (const auto& state : block.states) {
        const std::size_t constraint_number = state.constraints.size();
        for (std::size_t index = 0; index < constraint_number; ++index) {
          c.content << "<< \"," << margot::heel::generate_goal_identifier(state.name, index) << "\" ";
        }
      }
    }
    c.content << "<< std::endl;" << std::endl;
    c.content << "\t#endif // MARGOT_ENABLE_FILE_LOG" << std::endl;

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
    c.content << "c.manager.send_delta(\"{\\\"delta\\\":[";
    bool first = true;
    for (const auto& metric : block.metrics) {
      if(!first){
        c.content << ", ";
      }
      first = false;
      c.content << "{\\\"" << metric.name << "\\\":\"" << " + (c.manager.in_design_space_exploration() ? \"\\\"N/A\\\"\" : "
      "std::string(\"[\" + std::to_string(c.manager.get_mean<margot::OperatingPointSegments::METRICS,"
      << margot::heel::generate_field_getter(margot::heel::subject_kind::METRIC, metric.name, block.name)
      << ">()) + \",\" + std::to_string(c.monitors." + metric.monitor_name + ".last()) + \"]\")) + \"}";
    }
    c.content << "]}\");" << std::endl;
  }
  return c;
}

margot::heel::cpp_source_content log_content(const margot::heel::block_model& block) {
  margot::heel::cpp_source_content c;
  c.required_headers.emplace_back("mutex");
  c.required_headers.emplace_back("iostream");
  c.required_headers.emplace_back("string");
  c.required_headers.emplace_back("margot/enums.hpp");
  c.content << "auto& c = margot::" << block.name << "::context();" << std::endl;
  c.content << "const std::lock_guard<std::mutex> lock(c.context_mux);" << std::endl;

  // we need to emit the code that log on the standard output some information
  c.content << "#ifdef MARGOT_ENABLE_STDOUT_LOG" << std::endl;
  c.content << "std::cout << \".---------[ mARGOt log ]---------\" << std::endl;";
  c.content << "std::cout << \"|\" << std::endl << \"| Monitored values:\" << std::endl;" << std::endl;
  c.content << "std::cout << \"| \" ";
  for (const auto& monitor : block.monitors) {
    for (const auto& output_statistic : monitor.requested_statistics) {
      c.content << "<< \"[ " << monitor.name << "." << output_statistic << " = \" << std::string(c.monitors."
                << monitor.name << ".empty() ? std::string(\"N/A\") : std::to_string(c.monitors."
                << monitor.name << "." << output_statistic << "())) << \" ]\" ";
    }
  }
  c.content << "<< std::endl;" << std::endl;
  if (!block.knobs.empty()) {
    c.content << "std::cout << \"|\" << std::endl << \"| Features:\" << std::endl;" << std::endl;
    c.content << "std::cout << \"| \" ";
    for (const auto& feature : block.features.fields) {
      c.content << "<< \"[ " << feature.name << " = \" << c.features." << feature.name << " << \" ]\" ";
    }
    c.content << "<< std::endl;" << std::endl;
    c.content << "std::cout << \"|\" << std::endl << \"| Knob values:\" << std::endl;" << std::endl;
    c.content << "std::cout << \"| \" ";
    for (const auto& knob : block.knobs) {
      c.content << "<< \"[ " << knob.name << " = \" << c.knobs." << knob.name << " << \" ]\" ";
    }
    c.content << "<< std::endl;" << std::endl;
    c.content << "std::cout << \"|\" << std::endl << \"| Metric values:\" << std::endl;" << std::endl;
    c.content << "std::cout << \"| \" ";
    for (const auto& metric : block.metrics) {
      c.content << "<< \"[ " << metric.name
                << " = \" << std::string(c.manager.in_design_space_exploration() ? std::string(\"N/A\") : "
                   "std::to_string(c.manager.get_mean<margot::OperatingPointSegments::METRICS,"
                << margot::heel::generate_field_getter(margot::heel::subject_kind::METRIC, metric.name,
                                                       block.name)
                << ">())) << \" ]\" ";
    }
    c.content << "<< std::endl;" << std::endl;
    c.content << "std::cout << \"|\" << std::endl << \"| Constraint values:\" << std::endl;" << std::endl;
    c.content << "std::cout << \"| \" ";
    for (const auto& state : block.states) {
      const std::size_t constraint_number = state.constraints.size();
      for (std::size_t index = 0; index < constraint_number; ++index) {
        const std::string goal_name = margot::heel::generate_goal_identifier(state.name, index);
        c.content << "<< \"[ " << goal_name << " = \" << c.goals." << goal_name << ".get() << \" ]\" ";
      }
    }
    c.content << "<< std::endl;" << std::endl;
  }
  c.content << "std::cout << \"|________________________________\" << std::endl;" << std::endl;
  c.content << "#endif // MARGOT_ENABLE_STDOUT_LOG" << std::endl;

  // we need to emit the code that log on the file some information
  c.required_headers.emplace_back("chrono");
  c.content << "#ifdef MARGOT_ENABLE_FILE_LOG" << std::endl;
  c.content << "c.log_file << "
               "std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_"
               "since_epoch()).count();"
            << std::endl;
  for (const auto& monitor : block.monitors) {
    for (const auto& output_statistic : monitor.requested_statistics) {
      c.content << "c.log_file << ',' << std::string(c.monitors." << monitor.name
                << ".empty() ? std::string(\"N/A\") : std::to_string(c.monitors." << monitor.name << "."
                << output_statistic << "()));" << std::endl;
    }
  }
  if (!block.knobs.empty()) {
    for (const auto& metric : block.metrics) {
      c.content << "c.log_file << ',' << std::string(c.manager.in_design_space_exploration() ? "
                   "std::string(\"N/A\") : "
                   "std::to_string(c.manager.get_mean<margot::OperatingPointSegments::METRICS,"
                << margot::heel::generate_field_getter(margot::heel::subject_kind::METRIC, metric.name,
                                                       block.name)
                << ">()));" << std::endl;
    }
    for (const auto& feature : block.features.fields) {
      c.content << "c.log_file << ',' << c.features." << feature.name << ";" << std::endl;
    }
    for (const auto& knob : block.knobs) {
      c.content << "c.log_file << ',' << c.knobs." << knob.name << ";" << std::endl;
    }
    for (const auto& state : block.states) {
      const std::size_t constraint_number = state.constraints.size();
      for (std::size_t index = 0; index < constraint_number; ++index) {
        const std::string goal_name = margot::heel::generate_goal_identifier(state.name, index);
        c.content << "c.log_file << ',' << c.goals." << goal_name << ".get();" << std::endl;
      }
    }
  }
  c.content << "c.log_file << std::endl;" << std::endl;
  c.content << "#endif // MARGOT_ENABLE_FILE_LOG" << std::endl;
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
  const std::string init_signature = margot::heel::cpp_init_gen::signature(app);
  const std::string optional_coma = init_signature.empty() ? std::string("") : std::string(", ");
  c.content << "void init(" << init_signature << optional_coma << "const std::string& "
            << margot::heel::generate_log_file_name_identifier() << ") {" << std::endl;
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