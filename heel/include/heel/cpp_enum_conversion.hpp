#ifndef HEEL_CPP_ENUM_CONVERSION_HDR
#define HEEL_CPP_ENUM_CONVERSION_HDR

#include <stdexcept>
#include <string>

#include <heel/logger.hpp>
#include <heel/model_features.hpp>
#include <heel/model_state.hpp>

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
        margot::heel::error("cpp enum: unable to generate a cpp value for unknown feature comparison type");
        throw std::runtime_error("cpp enum:: unknown feature comparison type");
    };
    return "";
  }

  inline static std::string get(const rank_direction& direction) {
    switch (direction) {
      case rank_direction::MINIMIZE:
        return "margot::RankObjective::MINIMIZE";
        break;
      case rank_direction::MAXIMIZE:
        return "margot::RankObjective::MAXIMIZE";
        break;
      default:
        margot::heel::error("cpp enum: unable to generate a cpp value for unknown rank direction type");
        throw std::runtime_error("cpp enum:: unknown rank direction type");
    };
    return "";
  }

  inline static std::string get(const rank_type& type) {
    switch (type) {
      case rank_type::SIMPLE:
        return "margot::FieldComposer::SIMPLE";
        break;
      case rank_type::GEOMETRIC:
        return "margot::FieldComposer::GEOMETRIC";
        break;
      case rank_type::LINEAR:
        return "margot::FieldComposer::LINEAR";
        break;
      default:
        margot::heel::error("cpp enum: unable to generate a cpp value for unknown rank type");
        throw std::runtime_error("cpp enum:: unknown rank type");
    };
    return "";
  }

  inline static std::string get(const subject_kind& kind) {
    switch (kind) {
      case subject_kind::METRIC:
        return "margot::OperatingPointSegments::METRICS";
        break;
      case subject_kind::KNOB:
        return "margot::OperatingPointSegments::SOFTWARE_KNOBS";
        break;
      default:
        margot::heel::error("cpp enum: unable to generate a cpp value for unknown subject kind");
        throw std::runtime_error("cpp enum:: unknown subject kind");
    };
    return "";
  }

  inline static std::string get(const goal_comparison& cfun) {
    switch (cfun) {
      case goal_comparison::LESS_OR_EQUAL:
        return "margot::ComparisonFunctions::LESS_OR_EQUAL";
        break;
      case goal_comparison::GREATER_OR_EQUAL:
        return "margot::ComparisonFunctions::GREATER_OR_EQUAL";
        break;
      case goal_comparison::GREATER:
        return "margot::ComparisonFunctions::GREATER";
        break;
      case goal_comparison::LESS:
        return "margot::ComparisonFunctions::LESS";
        break;
      default:
        margot::heel::error("cpp enum: unable to generate a cpp value for unknown goal comparison");
        throw std::runtime_error("cpp enum:: unknown goal comparison");
    };
    return "";
  }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CPP_ENUM_CONVERSION_HDR