#include <cstdint>
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