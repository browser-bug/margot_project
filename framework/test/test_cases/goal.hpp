// MyTestSuite1.h
#include <cxxtest/TestSuite.h>

#include <string>

#include <margot/monitor.hpp>
#include <margot/goal.hpp>


class Goal : public CxxTest::TestSuite
{
  public:


    void test_check_values(void)
    {
      margot::Goal<int, margot::ComparisonFunctions::GREATER_OR_EQUAL> mygoal(2);

      TS_ASSERT((mygoal.check(2)));

      TS_ASSERT_DELTA((mygoal.relative_error<float>(2.5)), 0, 0.001);
      TS_ASSERT_DELTA((mygoal.relative_error<float>(1)), 0.5, 0.001);

      TS_ASSERT_DELTA((mygoal.absolute_error<float>(2.5)), 0, 0.001);
      TS_ASSERT_DELTA((mygoal.absolute_error<float>(1)), 1.0, 0.001);

    }

    void test_check_data_functions(void)
    {
      margot::Goal<int, margot::ComparisonFunctions::GREATER_OR_EQUAL> mygoal(2);
      margot::Monitor<float> monitor;

      TS_ASSERT((!mygoal.check<float, margot::DataFunctions::AVERAGE>(monitor)));

      monitor.push(2);
      TS_ASSERT((mygoal.check<float, margot::DataFunctions::AVERAGE>(monitor)));

      monitor.push(2.5);
      TS_ASSERT_DELTA((mygoal.relative_error<float, margot::DataFunctions::AVERAGE>(monitor)), 0, 0.001);
      monitor.push(1);
      TS_ASSERT_DELTA((mygoal.relative_error<float, margot::DataFunctions::AVERAGE>(monitor)), 0.5, 0.001);

      monitor.push(2.5);
      TS_ASSERT_DELTA((mygoal.absolute_error<float, margot::DataFunctions::AVERAGE>(monitor)), 0, 0.001);
      monitor.push(1);
      TS_ASSERT_DELTA((mygoal.absolute_error<float, margot::DataFunctions::AVERAGE>(monitor)), 1.0, 0.001);

    }


    void test_copy(void)
    {
      margot::Goal<float, margot::ComparisonFunctions::GREATER_OR_EQUAL> goal(1.5);
      auto mygoal(goal);

      TS_ASSERT((mygoal.check<int>(2)));

      TS_ASSERT_DELTA((mygoal.relative_error<float>(2.5)), 0, 0.001);
      TS_ASSERT_DELTA((mygoal.relative_error<float>(1)), 0.3333, 0.001);

      TS_ASSERT_DELTA((mygoal.absolute_error<float>(2.5)), 0, 0.001);
      TS_ASSERT_DELTA((mygoal.absolute_error<float>(1)), 0.5, 0.001);

      goal.set(2);

      TS_ASSERT_DELTA((mygoal.relative_error<float>(2.5)), 0, 0.001);
      TS_ASSERT_DELTA((mygoal.relative_error<float>(1)), 0.5, 0.001);

      TS_ASSERT_DELTA((mygoal.absolute_error<float>(2.5)), 0, 0.001);
      TS_ASSERT_DELTA((mygoal.absolute_error<float>(1)), 1.0, 0.001);
    }




};
