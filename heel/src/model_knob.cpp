#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include <heel/logger.hpp>
#include <heel/model/knob.hpp>

void margot::heel::validate(knob_model& model) {
  // check if the knob has a type
  if (model.type.empty()) {
    margot::heel::error("The knob \"", model.name, "\" must have a type");
    throw std::runtime_error("knob model: knob without a type");
  }

  // check if the type is a string, we must also have range of values, since we can use them to construct
  // translator to map a string to an integer
  if ((model.type.compare("string") == 0) && (model.values.empty())) {
    margot::heel::error("The type string for the knob \"", model.name,
                        "\" also requires the list of possible values");
    throw std::runtime_error("knob model: string enum missing");
  }
}
