#ifndef HEEL_PARAMETERS_HDR
#define HEEL_PARAMETERS_HDR

#include <string>

namespace margot {
namespace heel {

// this enum discriminates between different types of parameters for the representation of the
// extra-functional requirements. In particular, immediate values are C++ constants that are known at compile
// time (e.g. numbers), while variable values are C++ variables that are not known at compile time.
enum class parameter_types { IMMEDIATE, VARIABLE };

// this enum discriminates between different types of a value represented by the parameter.They are meant to
// be the C++ equivalent of "int", "float", "double"
enum class value_types { INTEGER, SINGLE_PRECISION_FLOAT, DOUBLE_PRECISION_FLOAT };

struct parameter {
  parameter_types type;
  std::string content;  // its semantic depends on the parameter type (e.g. a number or variable name)
  value_types value;
};

} // namespace heel
} // namespace margot

#endif  // HEEL_PARAMETERS_HDR