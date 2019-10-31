#ifndef HEEL_TYPER_HDR
#define HEEL_TYPER_HDR

#include <optional>
#include <stdexcept>
#include <string>

namespace margot {
namespace heel {

// this is an helper function that reverse the type aliasing for the current architecture (e.g.
// "int32_t"->"int"), this is required to limit the extension of possibilities.
std::string reverse_alias(const std::string& type_name);

// this is an helper function that returns true if the type a is "smaller" than b. If the two types belong to
// different categories (i.e. signed, unsigned and floating point), it return a "null" value
std::optional<bool> type_sorter(const std::string& a, const std::string& b);

// this struct is used to convert a c-type to a string with its name
template <typename T>
struct typer {
  static const std::string get(void) { return ""; }
};

template <>
struct typer<short int> {
  static const std::string get(void) { return "short int"; }
};

template <>
struct typer<unsigned short int> {
  static const std::string get(void) { return "unsigned short int"; }
};

template <>
struct typer<int> {
  static const std::string get(void) { return "int"; }
};

template <>
struct typer<unsigned int> {
  static const std::string get(void) { return "unsigned int"; }
};

template <>
struct typer<long int> {
  static const std::string get(void) { return "long int"; }
};

template <>
struct typer<unsigned long int> {
  static const std::string get(void) { return "unsigned long int"; }
};

template <>
struct typer<long long int> {
  static const std::string get(void) { return "long long int"; }
};

template <>
struct typer<unsigned long long int> {
  static const std::string get(void) { return "unsigned long long int"; }
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