#include <algorithm>
#include <stdexcept>

#include <heel/logger.hpp>
#include <heel/model/features.hpp>

void margot::heel::validate(features_model& model) {
  // the validation of this model depends if there are features or not
  if (!model.fields.empty()) {
    // we have features, therefore we need to ensure the uniqueness of the names and that they have the same
    // type
    const auto last_unique = std::unique(
        model.fields.begin(), model.fields.end(),
        [](const feature_model& a, const feature_model& b) { return a.name.compare(b.name) == 0; });
    if (last_unique != model.fields.end()) {
      margot::heel::error("Found ", std::distance(last_unique, model.fields.end()),
                          " duplicated feature field(s)");
      throw std::runtime_error("features model: duplicated features");
    }
    const std::string feature_type_str = model.fields.front().type;
    if (!std::all_of(model.fields.begin(), model.fields.end(),
                     [&feature_type_str](const feature_model& feature) {
                       return feature_type_str.compare(feature.type) == 0;
                     })) {
      margot::heel::error("All the feature fields must have the same type");
      throw std::runtime_error("features model: mismatch on feature types");
    }
  } else {
    // there are no input features, therefore it should have no distance
    if (model.distance_type != margot::heel::features_distance_type::NONE) {
      margot::heel::error("Set the feature distance type to \"", margot::heel::to_str(model.distance_type),
                          "\" even if there are no features");
      throw std::runtime_error("features model: inconsistent description");
    }
  }
}
