#include <vector>
#include <memory>

#include <cxxtest/TestSuite.h>

#include <margot/knowledge_base.hpp>
#include <margot/rank.hpp>

class Rank : public CxxTest::TestSuite
{

    using software_knob_geometry = margot::OperatingPointSegment< 2, margot::Data<int> >;
    using metrics_geometry = margot::OperatingPointSegment< 2, margot::Distribution<float> >;
    using MyOperatingPoint = margot::OperatingPoint< software_knob_geometry, metrics_geometry >;
    using OPStream = typename margot::RankInterface<MyOperatingPoint>::OPStream;

    OPStream op_list;

  public:


    void setUp( void )
    {
      op_list.emplace_back( new MyOperatingPoint(
      {
        {1, 2},
        {margot::Distribution<float>(3, 0.1), margot::Distribution<float>(4, 0.1)}
      }));
      op_list.emplace_back( new MyOperatingPoint(
      {
        {2, 3},
        {margot::Distribution<float>(5, 0.1), margot::Distribution<float>(6, 0.1)}
      }));
      op_list.emplace_back( new MyOperatingPoint(
      {
        {3, 4},
        {margot::Distribution<float>(7, 0.1), margot::Distribution<float>(8, 0.1)}
      }));
    }

    void tearDown( void )
    {
      op_list.clear();
    }


    void test_add( void )
    {
      std::shared_ptr< margot::RankInterface< MyOperatingPoint > > valid_ops;

      using first_metric_1sigma = margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 0, 1>;
      valid_ops.reset( new margot::Rank< MyOperatingPoint, margot::RankObjective::MAXIMIZE, margot::FieldComposer::SIMPLE, first_metric_1sigma >(1.0f));

      valid_ops->add({op_list[0]});

      auto result = valid_ops->best();
      TS_ASSERT(result)
      TS_ASSERT_EQUALS(result, op_list[0]);

      valid_ops->add({op_list[1]});
      result = valid_ops->best();
      TS_ASSERT_EQUALS(result, op_list[1]);

      valid_ops->add({op_list[2]});
      result = valid_ops->best();
      TS_ASSERT_EQUALS(result, op_list[2]);

      TS_ASSERT_EQUALS(result, valid_ops->best(op_list));
    }


    void test_remove( void )
    {
      std::shared_ptr< margot::RankInterface< MyOperatingPoint > > valid_ops;

      using first_metric_1sigma = margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 0, 1>;
      valid_ops.reset( new margot::Rank< MyOperatingPoint, margot::RankObjective::MAXIMIZE, margot::FieldComposer::SIMPLE, first_metric_1sigma >(1.0f));

      valid_ops->add(op_list);
      auto result = valid_ops->best();
      TS_ASSERT_EQUALS(result, op_list[2]);
      TS_ASSERT_EQUALS(result, valid_ops->best(op_list));

      TS_ASSERT(result)
      TS_ASSERT_EQUALS(result, op_list[2]);

      valid_ops->remove({op_list[2]});
      result = valid_ops->best();
      TS_ASSERT_EQUALS(result, op_list[1]);

      valid_ops->remove({op_list[1]});
      result = valid_ops->best();
      TS_ASSERT_EQUALS(result, op_list[0]);
    }
};
