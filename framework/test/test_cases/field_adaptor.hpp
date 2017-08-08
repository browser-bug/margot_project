#include <memory>

#include <cxxtest/TestSuite.h>

#include <margot/operating_point.hpp>
#include <margot/monitor.hpp>
#include <margot/field_adaptor.hpp>

class FieldAdaptor : public CxxTest::TestSuite
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

      // create the field adaptor
      std::shared_ptr<margot::FieldAdaptorInterface<MyOperatingPoint, float>> interface;
      interface.reset( new margot::FieldAdaptor<MyOperatingPoint,
                       margot::OperatingPointSegments::METRICS,
                       1, // index of metric
                       margot::DataFunctions::AVERAGE,
                       1, // inertia of the adaptor
                       float>(monitor));

      // check the coefficient error (expected 4 +- 0.1)
      monitor.push(4);
      interface->evaluate_error(my_op);
      TS_ASSERT_DELTA(interface->get_error_coefficient(), 1, delta);

      // check the coefficient error (expected 4 +- 0.1)
      monitor.push(4.09);
      interface->evaluate_error(my_op);
      TS_ASSERT_DELTA(interface->get_error_coefficient(), 1, delta);

      // check the coefficient error (expected 4 +- 0.1)
      monitor.push(4.1);
      interface->evaluate_error(my_op);
      TS_ASSERT_DELTA(interface->get_error_coefficient(), 1, delta);

      // check the coefficient error (expected 4 +- 0.1)
      monitor.push(4.11);
      interface->evaluate_error(my_op);
      TS_ASSERT_DELTA(interface->get_error_coefficient(), 0.973236, delta);


    }

};
