#include <algorithm>
#include <cstdint>

#include <heel/cpp_enum_conversion.hpp>
#include <heel/cpp_parser_gen.hpp>
#include <heel/generator_cpp_application_geometry_src.hpp>
#include <heel/generator_utils.hpp>
#include <heel/logger.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/model_knob.hpp>

namespace margot {
namespace heel {

cpp_source_content application_geometry_cpp_content(const application_model& app) {
  cpp_source_content c;

  // generate the content for each block managed by margot, within its namespace
  c.content << std::endl << "namespace margot {" << std::endl;
  std::for_each(app.blocks.begin(), app.blocks.end(), [&c](const block_model& block) {
    c.content << std::endl << "namespace " << block.name << " {" << std::endl << std::endl;

    // the parser struct is generated only if we need agora
    if (!block.agora.empty()) {
      c.required_headers.emplace_back("sstream");
      c.required_headers.emplace_back("string");
      c.required_headers.emplace_back("iterator");
      c.required_headers.emplace_back("boost/property_tree/ptree.hpp");
      c.required_headers.emplace_back("boost/property_tree/json_parser.hpp");
      c.required_headers.emplace_back("margot/application_geometry.hpp");

      // generate the function that parse the Operating Point list from a string representation
      c.content << "operating_point_container_type operating_point_parser::operator()(const std::string& "
                   "description_str) const {"
                << std::endl;

      // declare and parse the json using boost
      c.content << "\tstd::stringstream content_stream(description_str);" << std::endl;
      c.content << "\tboost::property_tree::ptree content;" << std::endl;
      c.content << "\tboost::property_tree::read_json(content_stream, content);" << std::endl;
      c.content << "\tboost::property_tree::ptree block_node = content.get_child(\"" << block.name << "\");"
                << std::endl;

      // before actually parsing the Operating Point and build the result, we need to take care of metrics
      // with distribution, since we can't grab just a value like with knobs and features. But we have to
      // iterate over the nodes, so we need to define a special lambda to do it
      c.content << "\tconst auto distribution_parser = [](const boost::property_tree::ptree& metric_node) {"
                << std::endl;
      c.content << "\t                                 const auto avg_elem_it = metric_node.begin();"
                << std::endl;
      c.content << "\t                                 const auto dev_elem_it = std::next(avg_elem_it);"
                << std::endl;
      c.content << "\t                                 return metrics_type(avg_elem_it->second.get<"
                << block.metrics_segment_type << ">(\"\"), dev_elem_it->second.get<"
                << block.metrics_segment_type << ">(\"\"));};" << std::endl;

      // now we can parse the property tree
      c.content << "\toperating_point_container_type result;" << std::endl;
      c.content << "\tfor(const auto& node_pair : block_node) {" << std::endl;

      // how to build the Operating Points list depends if we have features or not
      if (block.features.fields.empty()) {
        c.content << "\t\tresult.emplace_back(operating_point_type(";
      } else {
        c.content << "\t\tresult[{"
                  << join(block.features.fields.begin(), block.features.fields.end(), ", ",
                          [&block](const feature_model& field) {
                            return "node_pair.second.get<" + block.features.features_type + ">(\"features." +
                                   field.name + "\")";
                          })
                  << "}].emplace_back(operating_point_type(";
      }
      c.content << "{"
                << join(block.knobs.begin(), block.knobs.end(), ", ",
                        [&block](const knob_model& knob) {
                          return "node_pair.second.get<" + block.knobs_segment_type + ">(\"knobs." +
                                 knob.name + "\")";
                        })
                << "}, ";
      c.content << "{"
                << join(block.metrics.begin(), block.metrics.end(), ", ",
                        [&block](const metric_model& metric) -> std::string {
                          if (metric.distribution) {
                            return "distribution_parser(node_pair.second)";
                          } else {
                            return "metrics_type(node_pair.second.get<" + block.metrics_segment_type +
                                   ">(\"metrics." + metric.name + "\"), static_cast<" +
                                   block.metrics_segment_type + ">(0))";
                          }
                        })
                << "}));" << std::endl;

      c.content << "\t}" << std::endl;
      c.content << "\treturn result;" << std::endl;
      c.content << "}" << std::endl << std::endl;

      // generate the function that generate the string representation of an Operating Point list
      c.content << "std::string operating_point_parser::operator()("
                << cpp_parser_gen::signature(block.features.fields, block.knobs, block.metrics) << ") const {"
                << std::endl;
      c.content << "\treturn \"{\\\"" << block.name << "\\\":[{";
      if (!block.features.fields.empty()) {
        c.content << "\\\"features\\\":{"
                  << join(block.features.fields.begin(), block.features.fields.end(), ",",
                          [](const feature_model& field) {
                            return "\\\"" + field.name + "\\\":\" + std::to_string(" + field.name + ") + \"";
                          })
                  << "},";
      }
      c.content << "\\\"knobs\\\":{"
                << join(block.knobs.begin(), block.knobs.end(), ",",
                        [](const knob_model& knob) -> std::string {
                          if (knob.type.compare("string") == 0) {
                            return "\\\"" + knob.name + "\\\":\" + std::to_string(knob_" + knob.name +
                                   "_to_val(" + knob.name + ")) + \"";
                          } else {
                            return "\\\"" + knob.name + "\\\":\" + std::to_string(" + knob.name + ") + \"";
                          }
                        })
                << "},";
      c.content << "\\\"metrics\\\":{"
                << join(block.metrics.begin(), block.metrics.end(), ",",
                        [](const metric_model& metric) -> std::string {
                          const std::string prefix = "\\\"" + metric.name + "\\\":";
                          if (metric.distribution) {
                            return prefix + "[\" + std::to_string(" + metric.name + ") + \",0]";
                          }
                          return prefix + "\" + std::to_string(" + metric.name + ") + \"";
                        })
                << "}}]}\";" << std::endl;
      c.content << "}" << std::endl << std::endl;
    }

    c.content << "} // namespace " << block.name << std::endl << std::endl;
  });
  c.content << "} // namespace margot" << std::endl << std::endl;

  return c;
}

}  // namespace heel
}  // namespace margot