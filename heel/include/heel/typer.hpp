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

#ifndef HEEL_TYPER_HDR
#define HEEL_TYPER_HDR

#include <optional>
#include <stdexcept>
#include <string>

namespace margot {
namespace heel {

// this is an helper function that sanitize the input from the user regarding the type of a parameter, since
// the language ha a loose definition of fundamental types. In particular, it will:
//  - remove leading and trailing space
//  - remove the "std::" prefix (if any)
//  - convert the integer part to the aliased ones (e.g. "int" -> "int32_t")
//  - check if the type is supported, i.e. integers, floating points, and string. If it doesn't an exception
//  is thrown
// the sanitized input is the return value
std::string sanitize_type(const std::string& type_name);

// this is an helper function that returns true if the type a is "smaller" than b. If the two types belong to
// different categories (i.e. signed, unsigned and floating point), it return a "null" value
std::optional<bool> type_sorter(const std::string& a, const std::string& b);

// this is an helper function that checks whether a name is a valid c/c++ identifier
bool is_valid_identifier(const std::string& name);

// this struct is used to convert a c-type to a string with its name
template <typename T>
struct typer {
  static const std::string get(void) { return ""; }
};

template <>
struct typer<int8_t> {
  static const std::string get(void) { return "int8_t"; }
};

template <>
struct typer<uint8_t> {
  static const std::string get(void) { return "uint8_t"; }
};

template <>
struct typer<int16_t> {
  static const std::string get(void) { return "int16_t"; }
};

template <>
struct typer<uint16_t> {
  static const std::string get(void) { return "uint16_t"; }
};

template <>
struct typer<int32_t> {
  static const std::string get(void) { return "int32_t"; }
};

template <>
struct typer<uint32_t> {
  static const std::string get(void) { return "uint32_t"; }
};

template <>
struct typer<int64_t> {
  static const std::string get(void) { return "int64_t"; }
};

template <>
struct typer<uint64_t> {
  static const std::string get(void) { return "uint64_t"; }
};

template <>
struct typer<float> {
  static const std::string get(void) { return "float"; }
};

template <>
struct typer<double> {
  static const std::string get(void) { return "double"; }
};

template <>
struct typer<long double> {
  static const std::string get(void) { return "long double"; }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_TYPER_HDR