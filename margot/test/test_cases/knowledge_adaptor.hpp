#include <memory>

#include <cxxtest/TestSuite.h>

#include <margot/operating_point.hpp>
#include <margot/monitor.hpp>
#include <margot/knowledge_adaptor.hpp>

class KnowledgeAdaptor : public CxxTest::TestSuite
{

    using software_knob_geometry = margot::OperatingPointSegment< 2, margot::Data<int> >;
    using metrics_geometry = margot::OperatingPointSegment< 2, margot::Distribution<float> >;
    using MyOperatingPoint = margot::OperatingPoint< software_knob_geometry, metrics_geometry >;
    using OperatingPointPtr = std::shared_ptr< MyOperatingPoint >;

    OperatingPointPtr my_op;

  public:


    void setUp( void )
    {
      my_op.reset(new MyOperatingPoint(
      {
        {1, 2},
        {margot::Distribution<float>(3, 0.1), margot::Distribution<float>(4, 0.1)}
      }));
    }


    void test_creation( void )
    {
      static constexpr float delta = 0.0001;

      // create the monitor
      margot::Monitor<float> monitor;

      // create the knowledge adaptor;
      margot::KnowledgeAdaptor< MyOperatingPoint > adaptor;

      // check if all the fields adaptors are nullpointer
      TS_ASSERT(!(adaptor.get_field_adaptor<margot::OperatingPointSegments::SOFTWARE_KNOBS, 0>()));
      TS_ASSERT(!(adaptor.get_field_adaptor<margot::OperatingPointSegments::SOFTWARE_KNOBS, 1>()));
      TS_ASSERT(!(adaptor.get_field_adaptor<margot::OperatingPointSegments::METRICS, 0>()));
      TS_ASSERT(!(adaptor.get_field_adaptor<margot::OperatingPointSegments::METRICS, 1>()));

      // create the field adaptor for the second metric (with one of inertia)
      adaptor.emplace<margot::OperatingPointSegments::METRICS, 1, 1>(monitor);

      // get the new shiny field adaptor
      const auto my_adaptor = adaptor.get_field_adaptor<margot::OperatingPointSegments::METRICS, 1>();

      // check the validity of all the other adaptors
      TS_ASSERT(!(adaptor.get_field_adaptor<margot::OperatingPointSegments::SOFTWARE_KNOBS, 0>()));
      TS_ASSERT(!(adaptor.get_field_adaptor<margot::OperatingPointSegments::SOFTWARE_KNOBS, 1>()));
      TS_ASSERT(!(adaptor.get_field_adaptor<margot::OperatingPointSegments::METRICS, 0>()));
      TS_ASSERT(my_adaptor);

      // check the coefficient error (expected 4 +- 0.1)
      monitor.push(4);
      my_adaptor->evaluate_error(my_op);
      TS_ASSERT_DELTA(my_adaptor->get_error_coefficient(), 1, delta);

      // check the coefficient error (expected 4 +- 0.1)
      monitor.push(4.09);
      my_adaptor->evaluate_error(my_op);
      TS_ASSERT_DELTA(my_adaptor->get_error_coefficient(), 1, delta);

      // check the coefficient error (expected 4 +- 0.1)
      monitor.push(4.1);
      my_adaptor->evaluate_error(my_op);
      TS_ASSERT_DELTA(my_adaptor->get_error_coefficient(), 1, delta);

      // check the coefficient error (expected 4 +- 0.1)
      monitor.push(4.11);
      my_adaptor->evaluate_error(my_op);
      TS_ASSERT_DELTA(my_adaptor->get_error_coefficient(), 0.973236, delta);


    }

};
