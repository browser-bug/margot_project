/* mARGOt HEEL library
 * Copyright (C) 2018 Politecnico di Milano
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
#include <stdexcept>

#include <heel/logger.hpp>
#include <heel/model_features.hpp>
#include <heel/typer.hpp>

namespace margot {
namespace heel {

void validate(features_model& model) {
  // the validation of this model depends if there are features or not
  if (!model.fields.empty()) {
    // we have features, therefore we need to ensure the uniqueness of the names
    const auto last_unique = std::unique(
        model.fields.begin(), model.fields.end(),
        [](const feature_model& a, const feature_model& b) { return a.name.compare(b.name) == 0; });
    if (last_unique != model.fields.end()) {
      error("Found ", std::distance(last_unique, model.fields.end()), " duplicated feature field(s)");
      throw std::runtime_error("features model: duplicated features");
    }

    // now we can check each field of the features, if the type is supported and if the name is a valid c/c++
    // identifier
    std::for_each(model.fields.begin(), model.fields.end(), [](feature_model& field) {
      field.type = sanitize_type(field.type);
      if (field.type.compare("string") == 0) {
        error("The feature \"", field.name, "\" has a string type, this is not supported");
        throw std::runtime_error("features model: unsupported type");
      }
      if (!is_valid_identifier(field.name)) {
        error("The feature name \"", field.name, "\" is not a valid c/c++ identifier");
        throw std::runtime_error("features model: unsupported name");
      }
    });
  } else {
    // there are no input features, therefore it should have no distance
    if (model.distance_type != features_distance_type::NONE) {
      error("Set the feature distance type to \"", to_str(model.distance_type),
            "\" even if there are no features");
      throw std::runtime_error("features model: inconsistent description");
    }
  }
}

}  // namespace heel
}  // namespace margot