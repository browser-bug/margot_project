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
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>

#include "heel/parser_utils.hpp"
#include "heel/typer.hpp"

namespace margot {
namespace heel {

bool is_bool(std::string& str_value, const bool value) {
  boost::algorithm::to_lower(str_value);
  static const std::vector<std::string> true_values = {"1", "true", "yes", "enabled", "on", "high"};
  static const std::vector<std::string> false_values = {"0", "false", "no", "disabled", "off", "low"};
  return value ? std::any_of(true_values.begin(), true_values.end(),
                             [&str_value](const std::string& e) { return str_value.compare(e) == 0; })
               : std::any_of(false_values.begin(), false_values.end(),
                             [&str_value](const std::string& e) { return str_value.compare(e) == 0; });
}

// utility function that generates a sequence of values for a given type
template <class T>
std::vector<std::string> generate(const std::string& min, const std::string& max, const std::string& step) {
  std::vector<std::string> values;
  const T min_numeric = boost::lexical_cast<T>(min);
  const T max_numeric = boost::lexical_cast<T>(max);
  const T step_numeric = boost::lexical_cast<T>(step);
  for (T i = min_numeric; i < max_numeric; i += step_numeric) {
    values.emplace_back(std::to_string(i));
  }
  return values;
}

void compute_range(std::vector<std::string>& values, const boost::property_tree::ptree& range_node,
                   const std::string& value_type) {
  // now we have to do a kind of nasty stuff to generate the values... which is get them as strings, figure
  // out which is the correct types for them, and generate the values

  // get all the values that describe a range
  for (const auto& value_pair : range_node) {
    values.emplace_back(value_pair.second.get<std::string>("", ""));
  }

  // define the min, max, and step of the range (if specified)
  if (values.size() < static_cast<std::size_t>(2)) {
    throw std::runtime_error("compute range: unable to compute a range without a min and max value");
  }
  const std::string min = values[0];
  const std::string max = values[1];
  const std::string step = values.size() >= 3 ? values[2] : "1";  // default step is 1

  // now it begins the nightmare of generating the values according to the type of the knob
  const std::string type = sanitize_type(value_type);
  if (type.compare("int8_t") == 0) {
    values = generate<int8_t>(min, max, step);
  } else if (type.compare("uint8_t") == 0) {
    values = generate<uint8_t>(min, max, step);
  } else if (type.compare("int16_t") == 0) {
    values = generate<int16_t>(min, max, step);
  } else if (type.compare("uint16_t") == 0) {
    values = generate<uint16_t>(min, max, step);
  } else if (type.compare("int32_t") == 0) {
    values = generate<int32_t>(min, max, step);
  } else if (type.compare("uint32_t") == 0) {
    values = generate<uint32_t>(min, max, step);
  } else if (type.compare("int64_t") == 0) {
    values = generate<int64_t>(min, max, step);
  } else if (type.compare("uint64_t") == 0) {
    values = generate<uint64_t>(min, max, step);
  } else if (type.compare("float") == 0) {
    values = generate<float>(min, max, step);
  } else if (type.compare("double") == 0) {
    values = generate<double>(min, max, step);
  } else if (type.compare("long double") == 0) {
    values = generate<long double>(min, max, step);
  } else {
    throw std::runtime_error(" error_range: unabale to generate a range for type \"" + value_type + "\"");
  }
}

}  // namespace heel
}  // namespace margot