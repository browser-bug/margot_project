#include <algorithm>
#include <cstdint>
#include <sstream>

#include <heel/generator_cpp_application_geometry_hdr.hpp>
#include <heel/logger.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/model_knob.hpp>

margot::heel::cpp_source_content margot::heel::application_geometry_hpp_content(
    const application_model& app) {
  margot::heel::cpp_source_content c;

  // generate the content for each block managed by margot, within its namespace
  std::for_each(app.blocks.begin(), app.blocks.end(), [&c](const block_model& block) {
    c.content << std::endl << "namespace " << block.name << " {" << std::endl << std::endl;

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

    // now we need to create a type alias to define the Operating Point according to their geometry (if any)
    if (!block.knobs.empty()) {
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
                << block.knobs.size() << ", knobs_type>,margot::OperatingPointSegment<"
                << block.metrics.size() << ", metrics_type>>;" << std::endl;
    }

    c.content << "} // namespace " << block.name << std::endl;
  });

  return c;
}