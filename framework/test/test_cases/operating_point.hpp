#include <memory>

#include <cxxtest/TestSuite.h>

#include <margot/operating_point.hpp>

class OperatingPoint : public CxxTest::TestSuite
{
  public:


    void test_creation_point(void)
    {
      // defining the geometry of the Operating Points
      using KnobsType = margot::OperatingPointSegment< 2, margot::Data<int> >;
      using MetricsType = margot::OperatingPointSegment< 2, margot::Data<int> >;

      // defining the Operating Point
      margot::OperatingPoint<KnobsType, MetricsType> op({1, 2}, {3, 4});

      // check metrics
      TS_ASSERT_EQUALS(op.template get_knob_lower_bound<0>(), 1);
      TS_ASSERT_EQUALS(op.template get_knob_upper_bound<0>(), 1);
      TS_ASSERT_EQUALS(op.template get_knob_lower_bound<1>(), 2);
      TS_ASSERT_EQUALS(op.template get_knob_upper_bound<1>(), 2);

      // check software knobs
      TS_ASSERT_EQUALS(op.template get_metric_lower_bound<0>(), 3);
      TS_ASSERT_EQUALS(op.template get_metric_upper_bound<0>(), 3);
      TS_ASSERT_EQUALS(op.template get_metric_lower_bound<1>(), 4);
      TS_ASSERT_EQUALS(op.template get_metric_upper_bound<1>(), 4);
    }

    void test_creation_distribution1(void)
    {
      static constexpr float float_epsilon = 0.0001;

      // defining the geometry of the Operating Points
      using MetricsType = margot::OperatingPointSegment< 2, margot::Distribution<float> >;
      using KnobsType = margot::OperatingPointSegment< 2, margot::Distribution<float> >;

      // defining the Operating Point
      margot::OperatingPoint<KnobsType, MetricsType> op(
      {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(2, 0.1)},
      {margot::Distribution<float>(3, 0.1), margot::Distribution<float>(4, 0.1)}
      );

      // check metrics
      TS_ASSERT_DELTA(op.template get_knob_lower_bound<0>(), 0.7, float_epsilon);
      TS_ASSERT_DELTA(op.template get_knob_upper_bound<0>(), 1.3, float_epsilon);
      TS_ASSERT_DELTA(op.template get_knob_lower_bound<1>(), 1.7, float_epsilon);
      TS_ASSERT_DELTA(op.template get_knob_upper_bound<1>(), 2.3, float_epsilon);

      // check software knobs
      TS_ASSERT_DELTA(op.template get_metric_lower_bound<0>(), 2.7, float_epsilon);
      TS_ASSERT_DELTA(op.template get_metric_upper_bound<0>(), 3.3, float_epsilon);
      TS_ASSERT_DELTA(op.template get_metric_lower_bound<1>(), 3.7, float_epsilon);
      TS_ASSERT_DELTA(op.template get_metric_upper_bound<1>(), 4.3, float_epsilon);


    }

    void test_creation_distribution2(void)
    {
      static constexpr float float_epsilon = 0.0001;

      // defining the geometry of the Operating Points
      using MetricsType = margot::OperatingPointSegment< 2, margot::Distribution<float> >;
      using KnobsType = margot::OperatingPointSegment< 2, margot::Distribution<int> >;

      // defining the Operating Point
      margot::OperatingPoint<KnobsType, MetricsType> op(
      {margot::Distribution<int>(1, 0.1), margot::Distribution<int>(2, 0.1)},
      {margot::Distribution<float>(3, 0.1), margot::Distribution<float>(4, 0.1)}
      );

      // check metrics
      TS_ASSERT_DELTA(op.template get_knob_lower_bound<0>(), 0.7, float_epsilon);
      TS_ASSERT_DELTA(op.template get_knob_upper_bound<0>(), 1.3, float_epsilon);
      TS_ASSERT_DELTA(op.template get_knob_lower_bound<1>(), 1.7, float_epsilon);
      TS_ASSERT_DELTA(op.template get_knob_upper_bound<1>(), 2.3, float_epsilon);

      // check software knobs
      TS_ASSERT_DELTA(op.template get_metric_lower_bound<0>(), 2.7, float_epsilon);
      TS_ASSERT_DELTA(op.template get_metric_upper_bound<0>(), 3.3, float_epsilon);
      TS_ASSERT_DELTA(op.template get_metric_lower_bound<1>(), 3.7, float_epsilon);
      TS_ASSERT_DELTA(op.template get_metric_upper_bound<1>(), 4.3, float_epsilon);
    }

    void test_creation_mixed(void)
    {
      static constexpr float float_epsilon = 0.0001;

      // defining the geometry of the Operating Points
      using MetricsType = margot::OperatingPointSegment< 2, margot::Distribution<float> >;
      using KnobsType = margot::OperatingPointSegment< 2, margot::Data<int> >;

      // defining the Operating Point
      margot::OperatingPoint<KnobsType, MetricsType> op(
      {1, 2},
      {margot::Distribution<float>(3, 0.1), margot::Distribution<float>(4, 0.1)}
      );

      // check metrics
      TS_ASSERT_DELTA(op.template get_knob_lower_bound<0>(), 1, float_epsilon);
      TS_ASSERT_DELTA(op.template get_knob_upper_bound<0>(), 1, float_epsilon);
      TS_ASSERT_DELTA(op.template get_knob_lower_bound<1>(), 2, float_epsilon);
      TS_ASSERT_DELTA(op.template get_knob_upper_bound<1>(), 2, float_epsilon);

      // check software knobs
      TS_ASSERT_DELTA(op.template get_metric_lower_bound<0>(), 2.7, float_epsilon);
      TS_ASSERT_DELTA(op.template get_metric_upper_bound<0>(), 3.3, float_epsilon);
      TS_ASSERT_DELTA(op.template get_metric_lower_bound<1>(), 3.7, float_epsilon);
      TS_ASSERT_DELTA(op.template get_metric_upper_bound<1>(), 4.3, float_epsilon);
    }

    void test_getter_methods( void )
    {
      static constexpr float float_epsilon = 0.0001;

      // defining the geometry of the Operating Points
      using MetricsType = margot::OperatingPointSegment< 2, margot::Distribution<float> >;
      using KnobsType = margot::OperatingPointSegment< 2, margot::Data<int> >;
      using OperatingPointType = margot::OperatingPoint<KnobsType, MetricsType>;

      // defining the Operating Point
      std::shared_ptr< OperatingPointType > op;
      op.reset(new OperatingPointType(
      {1, 2},
      {margot::Distribution<float>(3, 0.1), margot::Distribution<float>(4, 0.1)}
               ));

      // check software knobs
      const auto k1_avg = margot::op_utils<OperatingPointType,
                 margot::OperatingPointSegments::SOFTWARE_KNOBS,
                 margot::BoundType::LOWER>::get<0, 0>(op);
      TS_ASSERT_EQUALS(k1_avg, 1);
      const auto k1_lb1 = margot::op_utils<OperatingPointType,
                 margot::OperatingPointSegments::SOFTWARE_KNOBS,
                 margot::BoundType::LOWER>::get<0, 1>(op);
      TS_ASSERT_EQUALS(k1_lb1, 1);
      const auto k1_lb2 = margot::op_utils<OperatingPointType,
                 margot::OperatingPointSegments::SOFTWARE_KNOBS,
                 margot::BoundType::LOWER>::get<0, 2>(op);
      TS_ASSERT_EQUALS(k1_lb2, 1);
      const auto k2_avg = margot::op_utils<OperatingPointType,
                 margot::OperatingPointSegments::SOFTWARE_KNOBS,
                 margot::BoundType::UPPER>::get<1, 0>(op);
      TS_ASSERT_EQUALS(k2_avg, 2);
      const auto k2_lb1 = margot::op_utils<OperatingPointType,
                 margot::OperatingPointSegments::SOFTWARE_KNOBS,
                 margot::BoundType::UPPER>::get<1, 1>(op);
      TS_ASSERT_EQUALS(k2_lb1, 2);
      const auto k2_lb2 = margot::op_utils<OperatingPointType,
                 margot::OperatingPointSegments::SOFTWARE_KNOBS,
                 margot::BoundType::UPPER>::get<1, 2>(op);
      TS_ASSERT_EQUALS(k2_lb2, 2);


      // check metrics
      const auto m1_avg = margot::op_utils<OperatingPointType,
                 margot::OperatingPointSegments::METRICS,
                 margot::BoundType::LOWER>::get<0, 0>(op);
      TS_ASSERT_DELTA(m1_avg, 3, float_epsilon);
      const auto m1_lb1 = margot::op_utils<OperatingPointType,
                 margot::OperatingPointSegments::METRICS,
                 margot::BoundType::LOWER>::get<0, 1>(op);
      TS_ASSERT_DELTA(m1_lb1, 2.9, float_epsilon);
      const auto m1_lb2 = margot::op_utils<OperatingPointType,
                 margot::OperatingPointSegments::METRICS,
                 margot::BoundType::LOWER>::get<0, 2>(op);
      TS_ASSERT_DELTA(m1_lb2, 2.8, float_epsilon);
      const auto m2_avg = margot::op_utils<OperatingPointType,
                 margot::OperatingPointSegments::METRICS,
                 margot::BoundType::UPPER>::get<1, 0>(op);
      TS_ASSERT_DELTA(m2_avg, 4, float_epsilon);
      const auto m2_lb1 = margot::op_utils<OperatingPointType,
                 margot::OperatingPointSegments::METRICS,
                 margot::BoundType::UPPER>::get<1, 1>(op);
      TS_ASSERT_DELTA(m2_lb1, 4.1, float_epsilon);
      const auto m2_lb2 = margot::op_utils<OperatingPointType,
                 margot::OperatingPointSegments::METRICS,
                 margot::BoundType::UPPER>::get<1, 2>(op);
      TS_ASSERT_DELTA(m2_lb2, 4.2, float_epsilon);
    }

};
