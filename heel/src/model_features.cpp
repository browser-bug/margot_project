#include <algorithm>
#include <stdexcept>

#include <heel/logger.hpp>
#include <heel/model_features.hpp>
#include <heel/typer.hpp>

void margot::heel::validate(features_model& model) {
  // the validation of this model depends if there are features or not
  if (!model.fields.empty()) {
    // we have features, therefore we need to ensure the uniqueness of the names
    const auto last_unique = std::unique(
        model.fields.begin(), model.fields.end(),
        [](const feature_model& a, const feature_model& b) { return a.name.compare(b.name) == 0; });
    if (last_unique != model.fields.end()) {
      margot::heel::error("Found ", std::distance(last_unique, model.fields.end()),
                          " duplicated feature field(s)");
      throw std::runtime_error("features model: duplicated features");
    }

    // to avoid any ambiguity between types, we need to sanitize them
    std::for_each(model.fields.begin(), model.fields.end(), [](feature_model& field) {
      field.type = margot::heel::sanitize_type(field.type);
      if (field.type.compare("string") == 0) {
        margot::heel::error("The feature \"", field.name, "\" has a string type, this is not supported");
        throw std::runtime_error("features model: unsupported type");
      }
    });
  } else {
    // there are no input features, therefore it should have no distance
    if (model.distance_type != margot::heel::features_distance_type::NONE) {
      margot::heel::error("Set the feature distance type to \"", margot::heel::to_str(model.distance_type),
                          "\" even if there are no features");
      throw std::runtime_error("features model: inconsistent description");
    }
  }
}
