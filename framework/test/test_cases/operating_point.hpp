// MyTestSuite1.h
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

};
