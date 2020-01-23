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