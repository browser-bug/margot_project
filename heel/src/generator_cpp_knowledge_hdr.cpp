#include <algorithm>

#include <heel/generator_cpp_knowledge_hdr.hpp>
#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>

namespace margot {
namespace heel {

cpp_source_content knowledge_hpp_content(application_model& app) {
  cpp_source_content c;
  c.required_headers.emplace_back("margot/application_geometry.hpp");
  c.content << "namespace margot {" << std::endl;
  std::for_each(app.blocks.begin(), app.blocks.end(), [&c](const block_model& block) {
    // define the protype of the function that adds the application knowledge (if required)
    if (!block.knobs.empty()) {
      c.content << "void add_application_knowledge(" << block.name << "::manager_type& manager);" << std::endl
                << std::endl;
    }
  });
  c.content << "} // namespace margot" << std::endl;
  return c;
}

}  // namespace heel
}  // namespace margot