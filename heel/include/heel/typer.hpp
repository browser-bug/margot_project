#ifndef HEEL_TYPER_HDR
#define HEEL_TYPER_HDR

#include <stdexcept>
#include <string>

namespace margot {
namespace heel {

template <typename T>
struct typer {
  static const std::string get(void) { throw std::runtime_error(" typer : unknown numeric type"); }
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