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

#ifndef HEEL_CONFIGURATION_FILE_HDR
#define HEEL_CONFIGURATION_FILE_HDR

#include <filesystem>
#include <string>

#include <boost/property_tree/ptree.hpp>

namespace margot {
namespace heel {

class configuration_file {
  boost::property_tree::ptree content;

 public:
  // I/O functions from file
  void load_json(const std::filesystem::path &file_path);
  void store_json(const std::filesystem::path &where) const;

  // I/O functions from std::string
  void load_json(const std::string &description);
  std::string to_json_string(void) const;

  // functions to retrieve the internal representation of the configuration file
  inline boost::property_tree::ptree &ptree(void) { return content; }
  inline const boost::property_tree::ptree &ptree(void) const { return content; }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CONFIGURATION_FILE_HDR