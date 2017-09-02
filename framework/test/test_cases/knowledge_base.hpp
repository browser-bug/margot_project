#include <vector>

#include <cxxtest/TestSuite.h>

#include <margot/knowledge_base.hpp>

class KnowledgeBase : public CxxTest::TestSuite
{

    using software_knob_geometry = margot::OperatingPointSegment< 2, margot::Data<int> >;
    using metrics_geometry = margot::OperatingPointSegment< 2, margot::Distribution<float> >;
    using MyOperatingPoint = margot::OperatingPoint< software_knob_geometry, metrics_geometry >;

    std::vector< MyOperatingPoint > op_list;

  public:


    void setUp( void )
    {
      op_list.push_back(
      {
        {1, 2},
        {margot::Distribution<float>(3, 0.1), margot::Distribution<float>(4, 0.1)}
      });
      op_list.push_back(
      {
        {2, 3},
        {margot::Distribution<float>(5, 0.1), margot::Distribution<float>(6, 0.1)}
      });
      op_list.push_back(
      {
        {3, 4},
        {margot::Distribution<float>(7, 0.1), margot::Distribution<float>(8, 0.1)}
      });
    }

    void tearDown( void )
    {
      op_list.clear();
    }


    void test_creation( void )
    {
      margot::Knowledge<MyOperatingPoint> kb;

      TS_ASSERT(kb.empty());
      TS_ASSERT_EQUALS(kb.size(), 0);
      TS_ASSERT_EQUALS(kb.begin(), kb.end());
    }

    void test_add( void )
    {
      margot::Knowledge<MyOperatingPoint> kb;

      // add the first op
      kb.add(op_list[0]);

      // check the result
      TS_ASSERT(!kb.empty());
      TS_ASSERT_EQUALS(kb.size(), 1);
      TS_ASSERT_DIFFERS(kb.begin(), kb.end());

      // add the second op
      kb.add(op_list[1]);

      // check the result
      TS_ASSERT(!kb.empty());
      TS_ASSERT_EQUALS(kb.size(), 2);
      TS_ASSERT_DIFFERS(kb.begin(), kb.end());

      // add the third op
      kb.add(op_list[2]);

      // check the result
      TS_ASSERT(!kb.empty());
      TS_ASSERT_EQUALS(kb.size(), 3);
      TS_ASSERT_DIFFERS(kb.begin(), kb.end());

      // check the sweep through the knowledge
      int counter = 0;

      for ( const auto& element : kb )
      {
        TS_ASSERT(element.second);
        ++counter;
      }

      TS_ASSERT_EQUALS(counter, kb.size());

      // try to add the same Operating Point
      margot::Knowledge<MyOperatingPoint>::OperatingPointPtr new_op =
        margot::Knowledge<MyOperatingPoint>::OperatingPointPtr( new MyOperatingPoint(
      {1, 2},
      {margot::Distribution<float>(3, 0.1), margot::Distribution<float>(4, 0.1)}
            ));
      auto result = kb.add(new_op);
      TS_ASSERT(!result);
      TS_ASSERT_EQUALS(kb.size(), counter);
    }


    void test_remove( void )
    {
      // initialize the knowledge base
      margot::Knowledge<MyOperatingPoint> kb;

      for ( const auto& op : op_list )
      {
        kb.add(op);
      }

      TS_ASSERT_EQUALS(kb.size(), op_list.size());

      // try to remove an existent Operating Point
      const auto removed_op = kb.remove(software_knob_geometry{1, 2});
      TS_ASSERT(removed_op);
      TS_ASSERT_EQUALS(removed_op->get_knob_lower_bound<0>(), 1);
      TS_ASSERT_EQUALS(kb.size(), op_list.size() - 1);

      // try to remove a non existent Operating Point
      const auto removed_op2 = kb.remove(software_knob_geometry{1, 2});
      TS_ASSERT(!removed_op2);
      TS_ASSERT_EQUALS(kb.size(), op_list.size() - 1);
    }

};
