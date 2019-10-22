#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>

#include "heel/parser/utils.hpp"
#include "heel/typer.hpp"

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

std::vector<std::string> margot::heel::compute_range(const boost::property_tree::ptree& range_node,
                                                     const std::string& value_type) {
  // now we have to do a kind of nasty stuff to generate the values... which is get them as strings, figure
  // out which is the correct types for them, and generate the values

  // get all the values that describe a range
  std::vector<std::string> range_values;
  for (const auto& value_pair : range_node) {
    range_values.emplace_back(value_pair.second.get<std::string>("", ""));
  }

  // define the min, max, and step of the range (if specified)
  if (range_values.size() < static_cast<std::size_t>(2)) {
    throw std::runtime_error("compute range: unable to compute a range without a min and max value");
  }
  const std::string min = range_values[0];
  const std::string max = range_values[1];
  const std::string step = range_values.size() >= 3 ? range_values[2] : "1";  // default step is 1

  // now it begins the nightmare of generating the values according to the type of the knob
  const std::string type = margot::heel::reverse_alias(value_type);
  if (type.compare("short int") == 0) {
    return generate<short int>(min, max, step);
  } else if (type.compare("unsigned short int") == 0) {
    return generate<unsigned short int>(min, max, step);
  } else if (type.compare("int") == 0) {
    return generate<int>(min, max, step);
  } else if (type.compare("unsigned int") == 0) {
    return generate<unsigned int>(min, max, step);
  } else if (type.compare("long int") == 0) {
    return generate<long int>(min, max, step);
  } else if (type.compare("unsigned long int") == 0) {
    return generate<unsigned long int>(min, max, step);
  } else if (type.compare("long long int") == 0) {
    return generate<long long int>(min, max, step);
  } else if (type.compare("unsigned long long int") == 0) {
    return generate<unsigned long long int>(min, max, step);
  } else if (type.compare("float") == 0) {
    return generate<float>(min, max, step);
  } else if (type.compare("double") == 0) {
    return generate<double>(min, max, step);
  } else if (type.compare("long double") == 0) {
    return generate<long double>(min, max, step);
  } else {
    throw std::runtime_error(" error_range: unabale to generate a range for type \"" + value_type + "\"");
  }
  return {};
}