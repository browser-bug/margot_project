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
  std::cout << line_formatter{}.format("  ERROR", arguments...);
}

}  // namespace heel
}  // namespace margot

#endif  // HEEL_LOGGER_HDR