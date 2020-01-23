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

#include <sstream>
#include <string>

#include <boost/property_tree/json_parser.hpp>

#include <heel/configuration_file.hpp>

namespace margot {
namespace heel {

void configuration_file::load_json(const std::filesystem::path &file_path) {
  boost::property_tree::read_json(static_cast<std::string>(file_path), content);
}

void configuration_file::load_json(const std::string &description) {
  std::stringstream content_stream(description);
  boost::property_tree::read_json(content_stream, content);
}

void configuration_file::store_json(const std::filesystem::path &where) const {
  boost::property_tree::write_json(static_cast<std::string>(where), content);
}

std::string configuration_file::to_json_string(void) const {
  std::stringstream content_stream;
  boost::property_tree::write_json(content_stream, content);
  return content_stream.str();
}

}  // namespace heel
}  // namespace margot