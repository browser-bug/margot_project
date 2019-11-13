#include <algorithm>
#include <cstdint>

#include <heel/cpp_enum_conversion.hpp>
#include <heel/cpp_interface_conversion.hpp>
#include <heel/generator_cpp_application_geometry_hdr.hpp>
#include <heel/generator_utils.hpp>
#include <heel/logger.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/model_knob.hpp>

margot::heel::cpp_source_content margot::heel::application_geometry_hpp_content(
    const application_model& app) {
  margot::heel::cpp_source_content c;

  // generate the content for each block managed by margot, within its namespace
  c.content << std::endl << "namespace margot {" << std::endl;
  std::for_each(app.blocks.begin(), app.blocks.end(), [&c](const block_model& block) {
    c.content << std::endl << "namespace " << block.name << "_utils {" << std::endl << std::endl;

    // generate the enums for the metrics and for the knobs
    c.required_headers.emplace_back("cstdint");
    c.content << "enum class knob: size_t {" << std::endl;
    size_t counter = 0;
    std::for_each(block.knobs.begin(), block.knobs.end(), [&c, &counter](const knob_model& knob) {
      c.content << '\t' << knob.name << " = " << counter++ << ',' << std::endl;
    });
    c.content << "};" << std::endl;
    counter = 0;
    c.content << std::endl << "enum class metric: size_t {" << std::endl;
    std::for_each(block.metrics.begin(), block.metrics.end(), [&c, &counter](const metric_model& metric) {
      c.content << '\t' << metric.name << " = " << counter++ << ',' << std::endl;
    });
    c.content << "};" << std::endl;
    c.content << std::endl;

    // generate the functions that translates a string knob to a numeric value and vice-versa
    std::for_each(block.knobs.begin(), block.knobs.end(), [&block, &c](const knob_model& knob) {
      if (knob.type.compare("string") == 0) {
        c.required_headers.emplace_back("string");
        c.required_headers.emplace_back("map");

        // define the function that translates from a string to a numeric value
        c.content << block.knobs_segment_type << " knob_" << knob.name
                  << "_to_val( const std::string& value) {" << std::endl;
        c.content << "\tstatic const std::map<std::string," << block.knobs_segment_type << "> translator = {"
                  << std::endl;
        size_t counter = 0;
        c.content << "\t\t"
                  << margot::heel::join(knob.values.begin(), knob.values.end(), ", ",
                                        [&counter](const std::string& value) {
                                          return "{\"" + value + "\", " + std::to_string(counter++) + "}";
                                        });
        c.content << std::endl << "\t};" << std::endl;
        c.content << "\treturn translator.at(value);" << std::endl << "}" << std::endl;

        // define also the reverse function to perform such task
        c.content << "std::string val_to_knob_" << knob.name << "( const " << block.knobs_segment_type
                  << " value) {" << std::endl;
        c.content << "\tstatic const std::map<" << block.knobs_segment_type << ", std::string> translator = {"
                  << std::endl;
        counter = 0;
        c.content << "\t\t"
                  << margot::heel::join(knob.values.begin(), knob.values.end(), ", ",
                                        [&counter](const std::string& value) {
                                          return "{" + std::to_string(counter++) + ", \"" + value + "\"}";
                                        });
        c.content << std::endl << "\t};" << std::endl;
        c.content << "\treturn translator.at(value);" << std::endl << "}" << std::endl;
      }
    });

    // now we need to create type aliase to define more easy-to-use types in the remainder of the interface
    if (!block.knobs.empty()) {
      // the first type aliases regards the definition of an operating point
      c.required_headers.emplace_back("margot/operating_point.hpp");
      const bool is_distribution =
          std::any_of(block.metrics.begin(), block.metrics.end(),
                      [](const metric_model& metric) { return metric.distribution; });
      std::string metric_type = "margot::";
      if (is_distribution) {
        metric_type += "Distribution<";
      } else {
        metric_type += "Data<";
      }
      metric_type += block.metrics_segment_type + ">";
      c.content << "using metrics_type = " << metric_type << ';' << std::endl;
      c.content << "using knobs_type = margot::Data<" << block.knobs_segment_type << ">;" << std::endl;
      c.content << "using operating_point_type = margot::OperatingPoint<margot::OperatingPointSegment<"
                << block.knobs.size() << ",knobs_type>,margot::OperatingPointSegment<" << block.metrics.size()
                << ",metrics_type>>;" << std::endl;

      // now we need to create a type alias to define the manager type for this specific block
      if (block.features.fields.empty()) {
        c.required_headers.emplace_back("margot/asrtm.hpp");
        c.content << "using manager_type = margot::Asrtm<operating_point_type>;" << std::endl;
      } else {
        c.required_headers.emplace_back("margot/da_asrtm.hpp");
        c.required_headers.emplace_back("margot/enums.hpp");
        c.content << "using manager_type = margot::DataAwareAsrtm<margot::Asrtm<operating_point_type>,"
                  << block.features.features_type << ","
                  << margot::heel::cpp_enum::get(block.features.distance_type) << ","
                  << margot::heel::join(block.features.fields.begin(), block.features.fields.end(), ",",
                                        [](const feature_model& field) {
                                          return margot::heel::cpp_enum::get(field.comparison);
                                        })
                  << ">;" << std::endl
                  << std::endl;
      }
    }

    // if agora is enabled, we need to define the struct that parses and generates a list of Operating
    // Points (the validation is already done, so if we have agora, we need to have knobs and metrics)
    if (block.agora.enabled) {
      // at this stage we need to work with strings
      c.required_headers.emplace_back("string");

      // introduce the type alias for the Operating Point container
      c.content
          << "using operating_point_container_type = typename manager_type::operating_point_container_type;"
          << std::endl;

      // define the functor that parses the operating point from string to object, and viceversa.
      c.content << "struct operating_point_parser {" << std::endl;
      c.content << "\toperating_point_container_type operator()(const std::string& description_str) const;"
                << std::endl;
      c.content << "\tstd::string operator()("
                << margot::heel::cpp_parameters::signature(block.features.fields, block.knobs, block.metrics)
                << ") const;" << std::endl;
      c.content << "};" << std::endl << std::endl;
    }

    c.content << "} // namespace " << block.name << "_utils" << std::endl << std::endl;
  });
  c.content << "} // namespace margot" << std::endl << std::endl;

  return c;
}