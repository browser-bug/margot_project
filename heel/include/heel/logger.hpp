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

#ifndef HEEL_LOGGER_HDR
#define HEEL_LOGGER_HDR

#include <iostream>
#include <sstream>
#include <string>

namespace margot {
namespace heel {

class line_formatter {
  std::ostringstream stream;

 public:
  template <class... T>
  inline std::string format(const std::string log_str, const T &... arguments) {
    stream << "[" << log_str << "] ";
    insert(arguments...);
    return stream.str();
  }

 private:
  template <class T, class... Y>
  inline void insert(const T &argument, const Y &... remainder) {
    stream << argument;
    insert(remainder...);
  }

  template <class T>
  inline void insert(const T &argument) {
    stream << argument << std::endl;
  }

  inline void insert(void) { stream << std::endl; }
};

template <class... T>
inline void info(const T &... arguments) {
  std::cout << line_formatter{}.format("   INFO", arguments...);
}

template <class... T>
inline void warning(const T &... arguments) {
  std::cout << line_formatter{}.format("WARNING", arguments...);
}

template <class... T>
inline void error(const T &... arguments) {
  std::cerr << line_formatter{}.format("  ERROR", arguments...);
}

}  // namespace heel
}  // namespace margot

#endif  // HEEL_LOGGER_HDR