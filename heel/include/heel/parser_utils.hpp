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

#ifndef HEEL_PARSER_UTILS_HDR
#define HEEL_PARSER_UTILS_HDR

#include <algorithm>
#include <string>
#include <vector>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

#include <heel/model_application.hpp>

namespace margot {
namespace heel {

// this is an utility function that retrieves the value of a node as a string, if it exists. If the node does
// not exist, it returns an empty string
inline void parse(std::string& value, const boost::property_tree::ptree& node) {
  value.assign(node.get<std::string>("", ""));
}

// this is an utility function that parses all the childs of the root node, with the name tag, if any, and it
// parses them. To avoid to specify a lambda that performs the parsing of the element T, we rely on the
// developer to make sure that the function margot::heel::parse of T is in scope, when this template is
// instantiated
template <class T>
void parse_list(std::vector<T>& models, const boost::property_tree::ptree& node, const std::string& tag) {
  const boost::optional<const boost::property_tree::ptree&> child = node.get_child_optional(tag);
  if (child) {
    for (const auto& node_pair : *child) {
      T element_model;
      margot::heel::parse(element_model, node_pair.second);  // first: node name as string
                                                             // second: the node content as ptree
      models.emplace_back(element_model);
    }
  }
}

// this is an utility function that parses a single child of the root node, with the name tag, if any, and it
// parses it. To avoid to specify a lambda that performs the parsing of the element T, we rely on the
// developer to make sure that the function margot::heel::parse of T is in scope, when this template is
// instantiated
template <class T>
void parse_element(T& model, const boost::property_tree::ptree& node, const std::string& tag) {
  const boost::optional<const boost::property_tree::ptree&> child = node.get_child_optional(tag);
  if (child) {
    margot::heel::parse(model, *child);
  }
}

// this is an utility function that checks if the string contents relates to an enum field. To avoid to
// specify a lambda that performs the parsing of the element T, we rely on the developer to make sure that the
// function margot::heel::to_str of T is in scope, when this template is instantiated. It return trues if it
// matches
template <typename T>
inline bool is_enum(std::string& str_value, const T enum_value) {
  boost::algorithm::to_lower(str_value);
  return str_value.compare(margot::heel::to_str(enum_value)) == 0;
}
bool is_bool(std::string& str_value, const bool value);

// this is an utility function that generates a list of values (as strings), starting from a range node
void compute_range(std::vector<std::string>& values, const boost::property_tree::ptree& range_node,
                   const std::string& value_type);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_UTILS_HDR