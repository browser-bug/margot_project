#include <memory>

#include <cxxtest/TestSuite.h>

#include <margot/evaluator.hpp>

class Evaluator : public CxxTest::TestSuite {
 public:
  void test_creation_point(void) {
    static constexpr float float_epsilon = 0.0001;

    // defining the geometry of the Operating Points
    using MetricsType = margot::OperatingPointSegment<2, margot::Distribution<float> >;
    using KnobsType = margot::OperatingPointSegment<2, margot::Data<int> >;
    using OperatingPointType = margot::OperatingPoint<KnobsType, MetricsType>;

    // defining the Operating Point
    std::shared_ptr<OperatingPointType> op;
    op.reset(new OperatingPointType(
        {1, 2}, {margot::Distribution<float>(3, 0.1), margot::Distribution<float>(4, 0.1)}));

    // create an evaluator for the first knob simple value
    using first_knob_avg =
        margot::OPField<margot::OperatingPointSegments::SOFTWARE_KNOBS, margot::BoundType::LOWER, 0, 0>;
    using evaluator1 = margot::Evaluator<OperatingPointType, margot::FieldComposer::SIMPLE, first_knob_avg>;
    const auto value1 = evaluator1::evaluate(op);
    TS_ASSERT_EQUALS(value1, 1);

    // create an evaluator to combine the first and the second knobs in a linear fashion
    using second_knob_avg =
        margot::OPField<margot::OperatingPointSegments::SOFTWARE_KNOBS, margot::BoundType::LOWER, 1, 0>;
    using evaluator2 =
        margot::Evaluator<OperatingPointType, margot::FieldComposer::LINEAR, first_knob_avg, second_knob_avg>;
    const auto value2 = evaluator2::evaluate(op, 1, 1);
    TS_ASSERT_EQUALS(value2, 3);

    // create an evaluator to combine the first and the second knobs in a geometric fashion
    using evaluator3 = margot::Evaluator<OperatingPointType, margot::FieldComposer::GEOMETRIC, first_knob_avg,
                                         second_knob_avg>;
    const auto value3 = evaluator3::evaluate(op, 1, 3);
    TS_ASSERT_DELTA(value3, 8, float_epsilon);
  }
};
