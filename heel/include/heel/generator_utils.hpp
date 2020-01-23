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

#ifndef HEEL_GENERATOR_UTILS_HDR
#define HEEL_GENERATOR_UTILS_HDR

#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace margot {
namespace heel {

// this utility function performs the equivalent of "''.join()" in python, but over an iterable of objects not
// only strings. The conversion from the object to a string is performed by the given functor.
// NOTE: we can't put the oneliner version, since we need to print the first element first
template <class iterator_type, class functor_type>
inline std::string join(const iterator_type& begin, const iterator_type& end, const std::string& separator,
                        const functor_type& functor) {
  std::string string_initial_element = begin == end ? std::string() : functor(*begin);
  return begin != end ? string_initial_element +
                            std::accumulate(std::next(begin), end, std::string{},
                                            [&separator, &functor](
                                                const std::string& partial_result,
                                                const typename iterator_type::value_type& new_element) {
                                              return partial_result + separator + functor(new_element);
                                            })
                      : string_initial_element;
}

// this struct represents a piece of content in cpp source/header file, plus all the headers that it requires
struct cpp_source_content {
  std::ostringstream content;
  std::vector<std::string> required_headers;
};

// this function appends a cpp source content to another one, with the possibility to specify a prefix
void append(cpp_source_content& destination, const cpp_source_content& source,
            const std::string& prefix = "");

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_UTILS_HDR