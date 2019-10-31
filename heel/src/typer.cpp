#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include <heel/typer.hpp>

// i know that this is ugly, but there is no other way
std::string margot::heel::reverse_alias(const std::string& type_name) {
  if (type_name.compare("int8_t") == 0) {
    return margot::heel::typer<int8_t>::get();
  } else if (type_name.compare("int16_t") == 0) {
    return margot::heel::typer<int16_t>::get();
  } else if (type_name.compare("int32_t") == 0) {
    return margot::heel::typer<int32_t>::get();
  } else if (type_name.compare("int64_t") == 0) {
    return margot::heel::typer<int64_t>::get();
  } else if (type_name.compare("int_fast8_t") == 0) {
    return margot::heel::typer<int_fast8_t>::get();
  } else if (type_name.compare("int_fast16_t") == 0) {
    return margot::heel::typer<int_fast16_t>::get();
  } else if (type_name.compare("int_fast32_t") == 0) {
    return margot::heel::typer<int_fast32_t>::get();
  } else if (type_name.compare("int_fast64_t") == 0) {
    return margot::heel::typer<int_fast64_t>::get();
  } else if (type_name.compare("int_least8_t") == 0) {
    return margot::heel::typer<int_least8_t>::get();
  } else if (type_name.compare("int_least16_t") == 0) {
    return margot::heel::typer<int_least16_t>::get();
  } else if (type_name.compare("int_least32_t") == 0) {
    return margot::heel::typer<int_least32_t>::get();
  } else if (type_name.compare("int_least64_t") == 0) {
    return margot::heel::typer<int_least64_t>::get();
  } else if (type_name.compare("uint8_t") == 0) {
    return margot::heel::typer<uint8_t>::get();
  } else if (type_name.compare("uint16_t") == 0) {
    return margot::heel::typer<uint16_t>::get();
  } else if (type_name.compare("uint32_t") == 0) {
    return margot::heel::typer<uint32_t>::get();
  } else if (type_name.compare("uint64_t") == 0) {
    return margot::heel::typer<uint64_t>::get();
  } else if (type_name.compare("uint_fast8_t") == 0) {
    return margot::heel::typer<uint_fast8_t>::get();
  } else if (type_name.compare("uint_fast16_t") == 0) {
    return margot::heel::typer<uint_fast16_t>::get();
  } else if (type_name.compare("uint_fast32_t") == 0) {
    return margot::heel::typer<uint_fast32_t>::get();
  } else if (type_name.compare("uint_fast64_t") == 0) {
    return margot::heel::typer<uint_fast64_t>::get();
  } else if (type_name.compare("uint_least8_t") == 0) {
    return margot::heel::typer<uint_least8_t>::get();
  } else if (type_name.compare("uint_least16_t") == 0) {
    return margot::heel::typer<uint_least16_t>::get();
  } else if (type_name.compare("uint_least32_t") == 0) {
    return margot::heel::typer<uint_least32_t>::get();
  } else if (type_name.compare("uint_least64_t") == 0) {
    return margot::heel::typer<uint_least64_t>::get();
  } else if (type_name.compare("intmax_t") == 0) {
    return margot::heel::typer<intmax_t>::get();
  } else if (type_name.compare("intptr_t") == 0) {
    return margot::heel::typer<intptr_t>::get();
  } else if (type_name.compare("uintmax_t") == 0) {
    return margot::heel::typer<uintmax_t>::get();
  } else if (type_name.compare("uintptr_t") == 0) {
    return margot::heel::typer<uintptr_t>::get();
  } else
    return type_name;
}

// this is a map that contains the promotions of ctypes
static const std::map<std::string, int> ctype_promotions_signed = {
    {"short int", 0},
    {"int", 1},
    {"long int", 2},
    {"long long int", 3},
};
static const std::map<std::string, int> ctype_promotions_unsigned = {
    {"unsigned short int", 0},
    {"unsigned int", 1},
    {"unsigned long int", 2},
    {"unsigned long long int", 3},
};
static const std::map<std::string, int> ctype_promotions_fp = {
    {"float", 0}, {"double", 1}, {"long double", 2}};

std::optional<bool> margot::heel::type_sorter(const std::string& a, const std::string& b) {
  // remove the type alias to limit the number of options
  const auto unaliased_a = margot::heel::reverse_alias(a);
  const auto unaliased_b = margot::heel::reverse_alias(b);

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