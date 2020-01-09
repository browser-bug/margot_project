#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iterator>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>

#include <boost/algorithm/string.hpp>

#include <heel/logger.hpp>
#include <heel/typer.hpp>

namespace margot {
namespace heel {

bool is_valid_identifier(const std::string& name) {
  if (name.empty()) return false;
  if (!((std::isalpha(name[0]) || (name[0] == static_cast<std::string::value_type>('_'))))) return false;
  if (std::any_of(std::next(name.begin()), name.end(), [](const std::string::value_type c) {
        return !(std::isalnum(c) || c == static_cast<std::string::value_type>('_'));
      })) {
    return false;
  }
  return true;
}

// i know that this is ugly, but there is no other way
std::string sanitize_type(const std::string& type_name) {
  // given the extreme flexibility to define a type in c using the fundamental types, we need to perform
  // search string to guess the actual type. Moreover, we need to take into account the standard type aliases
  // for defining the variable size in an architecture independent way

  // first of all, we need to sanitize the input from extra spaces
  std::string stripped_type_name;
  std::unique_copy(type_name.begin(), type_name.end(),
                   std::back_insert_iterator<std::string>(stripped_type_name),
                   [](const char a, const char b) { return std::isspace(a) && std::isspace(b); });
  boost::trim(stripped_type_name);

  // now we have to remove the std prefix, since it can interfere with c-interface generation
  if (stripped_type_name.rfind("std::", 0) == 0) {
    stripped_type_name.erase(0, 5);
  }

  // if the type is a string, we can bail out
  if (stripped_type_name.compare("string") == 0) {
    return "string";
  }

  // to have a more compact comparison, we need create an alias of the type name with a shorter name
  const std::string& t = stripped_type_name;

  // now we can determine which type is it
  if ((t.compare("short int") == 0) || (t.compare("short") == 0)) {
    return typer<short int>::get();
  } else if (t.compare("int8_t") == 0) {
    return typer<int8_t>::get();
  } else if (t.compare("int16_t") == 0) {
    return typer<int16_t>::get();
  } else if (t.compare("int32_t") == 0) {
    return typer<int32_t>::get();
  } else if (t.compare("int64_t") == 0) {
    return typer<int64_t>::get();
  } else if (t.compare("uint8_t") == 0) {
    return typer<uint8_t>::get();
  } else if (t.compare("uint16_t") == 0) {
    return typer<uint16_t>::get();
  } else if (t.compare("uint32_t") == 0) {
    return typer<uint32_t>::get();
  } else if (t.compare("uint64_t") == 0) {
    return typer<uint64_t>::get();
  } else if (t.compare("char") == 0) {
    return typer<char>::get();
  } else if (t.compare("int") == 0) {
    return typer<int>::get();
  } else if ((t.compare("long int") == 0) || (t.compare("long") == 0)) {
    return typer<long int>::get();
  } else if ((t.compare("long long int") == 0) || (t.compare("long long") == 0)) {
    return typer<long long int>::get();
  } else if (t.compare("int_fast8_t") == 0) {
    return typer<int_fast8_t>::get();
  } else if (t.compare("int_fast16_t") == 0) {
    return typer<int_fast16_t>::get();
  } else if (t.compare("int_fast32_t") == 0) {
    return typer<int_fast32_t>::get();
  } else if (t.compare("int_fast64_t") == 0) {
    return typer<int_fast64_t>::get();
  } else if (t.compare("int_least8_t") == 0) {
    return typer<int_least8_t>::get();
  } else if (t.compare("int_least16_t") == 0) {
    return typer<int_least16_t>::get();
  } else if (t.compare("int_least32_t") == 0) {
    return typer<int_least32_t>::get();
  } else if (t.compare("int_least64_t") == 0) {
    return typer<int_least64_t>::get();
  } else if ((t.compare("unsigned short int") == 0) || (t.compare("unsigned short") == 0) ||
             (t.compare("short unsigned int") == 0) || (t.compare("short unsigned") == 0)) {
    return typer<unsigned short int>::get();
  } else if ((t.compare("unsigned char") == 0) || (t.compare("char unsigned") == 0)) {
    return typer<unsigned char>::get();
  } else if ((t.compare("unsigned int") == 0) || (t.compare("unsigned") == 0)) {
    return typer<unsigned int>::get();
  } else if ((t.compare("unsigned long int") == 0) || (t.compare("unsigned long") == 0) ||
             (t.compare("long unsigned int") == 0) || (t.compare("long unsigned") == 0)) {
    return typer<unsigned long int>::get();
  } else if ((t.compare("unsigned long long int") == 0) || (t.compare("unsigned long long") == 0) ||
             (t.compare("long long unsigned int") == 0) || (t.compare("long long unsigned") == 0) ||
             (t.compare("long unsigned long") == 0) || (t.compare("long unsigned long int") == 0)) {
    return typer<unsigned long long int>::get();
  } else if (t.compare("uint_fast8_t") == 0) {
    return typer<uint_fast8_t>::get();
  } else if (t.compare("uint_fast16_t") == 0) {
    return typer<uint_fast16_t>::get();
  } else if (t.compare("uint_fast32_t") == 0) {
    return typer<uint_fast32_t>::get();
  } else if (t.compare("uint_fast64_t") == 0) {
    return typer<uint_fast64_t>::get();
  } else if (t.compare("uint_least8_t") == 0) {
    return typer<uint_least8_t>::get();
  } else if (t.compare("uint_least16_t") == 0) {
    return typer<uint_least16_t>::get();
  } else if (t.compare("uint_least32_t") == 0) {
    return typer<uint_least32_t>::get();
  } else if (t.compare("uint_least64_t") == 0) {
    return typer<uint_least64_t>::get();
  } else if (t.compare("intmax_t") == 0) {
    return typer<intmax_t>::get();
  } else if (t.compare("intptr_t") == 0) {
    return typer<intptr_t>::get();
  } else if (t.compare("uintmax_t") == 0) {
    return typer<uintmax_t>::get();
  } else if (t.compare("uintptr_t") == 0) {
    return typer<uintptr_t>::get();
  } else if (t.compare("float") == 0) {
    return typer<float>::get();
  } else if (t.compare("double") == 0) {
    return typer<double>::get();
  } else if ((t.compare("long double") == 0) || (t.compare("double long") == 0)) {
    return typer<long double>::get();
  } else {
    // if we reach this statement, it means that the type is unknown so we need to throw an expection
    error("Unable to understand the type \"", type_name, "\"");
    throw std::runtime_error("type sanitizer: unknown type");
  }
}

// this is a map that contains the promotions of ctypes
static const std::map<std::string, int> ctype_promotions_signed = {
    {"int8_t", 0},
    {"int16_t", 1},
    {"int32_t", 2},
    {"int64_t", 3},
};
static const std::map<std::string, int> ctype_promotions_unsigned = {
    {"uint8_t", 0},
    {"uint16_t", 1},
    {"uint32_t", 2},
    {"uint64_t", 3},
};
static const std::map<std::string, int> ctype_promotions_fp = {
    {"float", 0}, {"double", 1}, {"long double", 2}};

std::optional<bool> type_sorter(const std::string& a, const std::string& b) {
  // remove the type alias to limit the number of options
  const auto unaliased_a = sanitize_type(a);
  const auto unaliased_b = sanitize_type(b);

  // declare a lambda that compares two type only if they belong to the same set
  const auto check_lambda = [&unaliased_a,
                             &unaliased_b](const std::map<std::string, int>& map) -> std::optional<bool> {
    const auto a_it = map.find(unaliased_a);
    const auto b_it = map.find(unaliased_b);
    if ((a_it != map.cend()) && (b_it != map.cend())) {
      return a_it->second < b_it->second;
    }
    return {};
  };

  // perform the comparisons
  const auto res_signed = check_lambda(ctype_promotions_signed);
  if (res_signed) {
    return *res_signed;
  }
  const auto res_unsigned = check_lambda(ctype_promotions_unsigned);
  if (res_unsigned) {
    return *res_unsigned;
  }
  const auto res_fp = check_lambda(ctype_promotions_fp);
  if (res_fp) {
    return *res_fp;
  }
  return {};
}

}  // namespace heel
}  // namespace margot