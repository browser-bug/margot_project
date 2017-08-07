// MyTestSuite1.h
#include <cxxtest/TestSuite.h>

#include <string>

#include <margot/circular_buffer.hpp>
#include <margot/statistical_provider.hpp>
#include <margot/monitor.hpp>

class Monitor : public CxxTest::TestSuite
{
  public:


    void test_creation_buffer(void)
    {

      margot::CircularBuffer<float> bufferf(1);
      margot::CircularBuffer<int> bufferi(1);
      margot::CircularBuffer<std::string> buffers(2);
    }


    void test_addition1(void)
    {
      margot::CircularBuffer<int> buffer(1);
      TS_ASSERT(buffer.empty());
      TS_ASSERT(!buffer.full());

      buffer.push(1);
      TS_ASSERT(!buffer.empty());
      TS_ASSERT(buffer.full());
      TS_ASSERT_EQUALS(buffer.last(), 1);

      buffer.push(2);
      TS_ASSERT(!buffer.empty());
      TS_ASSERT(buffer.full());
      TS_ASSERT_EQUALS(buffer.last(), 2);

      buffer.push(3);
      TS_ASSERT(!buffer.empty());
      TS_ASSERT(buffer.full());
      TS_ASSERT_EQUALS(buffer.last(), 3);

      buffer.clear();
      TS_ASSERT(buffer.empty());
      TS_ASSERT(!buffer.full());

      buffer.push(1);
      TS_ASSERT(!buffer.empty());
      TS_ASSERT(buffer.full());
      TS_ASSERT_EQUALS(buffer.last(), 1);
    }

    void test_addition3(void)
    {
      margot::CircularBuffer<int> buffer(3);
      TS_ASSERT(buffer.empty());
      TS_ASSERT(!buffer.full());

      buffer.push(1);
      TS_ASSERT(!buffer.empty());
      TS_ASSERT(!buffer.full());
      TS_ASSERT_EQUALS(buffer.last(), 1);

      buffer.push(2);
      TS_ASSERT(!buffer.empty());
      TS_ASSERT(!buffer.full());
      TS_ASSERT_EQUALS(buffer.last(), 2);

      buffer.push(3);
      TS_ASSERT(!buffer.empty());
      TS_ASSERT(buffer.full());
      TS_ASSERT_EQUALS(buffer.last(), 3);

      buffer.clear();
      TS_ASSERT(buffer.empty());
      TS_ASSERT(!buffer.full());

      buffer.push(1);
      TS_ASSERT(!buffer.empty());
      TS_ASSERT(!buffer.full());
      TS_ASSERT_EQUALS(buffer.last(), 1);
    }

    void test_statistical_creation( void )
    {
      static constexpr float delta = 0.0001;
      margot::StatisticalProvider<int> buffer(3);

      buffer.push(1);
      TS_ASSERT_DELTA(buffer.average(), 1, delta);
      TS_ASSERT_DELTA(buffer.standard_deviation(), 0, delta);
      TS_ASSERT_DELTA(buffer.min(), 1, delta);
      TS_ASSERT_DELTA(buffer.max(), 1, delta);

      buffer.push(2);
      TS_ASSERT_DELTA(buffer.average(), 1.5f, delta);
      TS_ASSERT_DELTA(buffer.standard_deviation(), 0.70711, delta);
      TS_ASSERT_DELTA(buffer.min(), 1, delta);
      TS_ASSERT_DELTA(buffer.max(), 2, delta);

      buffer.push(3);
      TS_ASSERT_DELTA(buffer.average(), 2, delta);
      TS_ASSERT_DELTA(buffer.standard_deviation(), 1, delta);
      TS_ASSERT_DELTA(buffer.min(), 1, delta);
      TS_ASSERT_DELTA(buffer.max(), 3, delta);
    }

    void test_monitor( void )
    {
      margot::Monitor<int> my_monitor(1);
      my_monitor.push(3);
    }

};
