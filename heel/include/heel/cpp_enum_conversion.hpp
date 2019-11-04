#ifndef HEEL_CPP_ENUM_CONVERSION_HDR
#define HEEL_CPP_ENUM_CONVERSION_HDR

#include <stdexcept>
#include <string>

#include <heel/logger.hpp>
#include <heel/model_features.hpp>

namespace margot {
namespace heel {

// this is an helper struct to convert an enum to a cpp equivalent
struct cpp_enum {
  inline static std::string get(const features_distance_type& type) {
    switch (type) {
      case features_distance_type::EUCLIDEAN:
        return "margot::FeatureDistanceType::EUCLIDEAN";
        break;
      case features_distance_type::NORMALIZED:
        return "margot::FeatureDistanceType::NORMALIZED";
        break;
      default:
        margot::heel::error("cpp enum: unable to generate a cpp value for unknown feature distance type");
        throw std::runtime_error("cpp enum:: unknown feature distance type");
    };
    return "";
  }

  inline static std::string get(const distance_comparison_type& distance) {
    switch (distance) {
      case distance_comparison_type::LESS_OR_EQUAL:
        return "margot::FeatureComparison::LESS_OR_EQUAL";
        break;
      case distance_comparison_type::GREATER_OR_EQUAL:
        return "margot::FeatureComparison::GREATER_OR_EQUAL";
        break;
      case distance_comparison_type::DONT_CARE:
        return "margot::FeatureComparison::DONT_CARE";
        break;
      default:
        margot::heel::error("cpp enum: unable to generate a cpp value for unknown feature comparison types");
        throw std::runtime_error("cpp enum:: unknown feature comparison type");
    };
    return "";
  }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CPP_ENUM_CONVERSION_HDR