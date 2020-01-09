/* mARGOt HEEL library
 * Copyright (C) 2018 Politecnico di Milano
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
#include <sstream>
#include <string>

#include <heel/generator_utils.hpp>

namespace margot {
namespace heel {

void append(cpp_source_content& destination, const cpp_source_content& source, const std::string& prefix) {
  std::copy(source.required_headers.begin(), source.required_headers.end(),
            std::back_inserter(destination.required_headers));
  std::istringstream content(source.content.str());
  for (std::string line; std::getline(content, line); /* already handled */) {
    destination.content << prefix << line << std::endl;
  }
}

}  // namespace heel
}  // namespace margot