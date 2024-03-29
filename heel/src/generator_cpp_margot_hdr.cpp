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

#include <heel/cpp_init_gen.hpp>
#include <heel/cpp_push_monitor_gen.hpp>
#include <heel/cpp_start_monitor_gen.hpp>
#include <heel/cpp_stop_monitor_gen.hpp>
#include <heel/cpp_update_gen.hpp>
#include <heel/cpp_utils.hpp>
#include <heel/generator_cpp_margot_hdr.hpp>
#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>

namespace margot {
namespace heel {

cpp_source_content margot_hpp_content(const application_model& app) {
  cpp_source_content c;

  // generate the preamble of the header
  c.required_headers.emplace_back("string");
  c.required_headers.emplace_back("cstdint");
  c.required_headers.emplace_back("margot/managers_definition.hpp");
  c.content << "namespace margot {" << std::endl << std::endl;

  // define the prototype of the global initialization function
  const std::string init_signature = cpp_init_gen::signature(app);
  const std::string optional_coma = init_signature.empty() ? std::string("") : std::string(", ");
  c.content << "void init(" << init_signature << optional_coma << "const std::string& "
            << generate_log_file_name_identifier() << " = \"\");" << std::endl;

  // loop over each block to generate the block-specific functions
  std::for_each(app.blocks.begin(), app.blocks.end(), [&c](const block_model& block) {
    c.content << std::endl << "namespace " << block.name << " {" << std::endl;
    c.content << "bool update(" << cpp_update_gen::signature(block) << ");" << std::endl;
    c.content << "void start_monitors(" << cpp_start_monitor_gen::signature(block) << ");" << std::endl;
    c.content << "void stop_monitors(" << cpp_stop_monitor_gen::signature(block) << ");" << std::endl;
    c.content << "void push_custom_monitor_values(" << cpp_push_monitor_gen::signature(block) << ");"
              << std::endl;
    c.content << "void log(void);" << std::endl;
    c.content << "inline margot::" << block.name << "::data& context(void) {" << std::endl;
    c.content << "\treturn margot::" << block.name << "::data::get_instance();" << std::endl;
    c.content << "}" << std::endl;
    c.content << "} // namespace " << block.name << std::endl;
  });

  // generate the trailer of the header
  c.content << std::endl << "} // namespace margot" << std::endl;

  return c;
}

}  // namespace heel
}  // namespace margot