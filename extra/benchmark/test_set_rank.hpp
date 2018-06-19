#ifndef MARGOT_TEST_SET_RANK_HDR
#define MARGOT_TEST_SET_RANK_HDR


#include "margot/asrtm.hpp"

#include "evaluator.hpp"



template< bool worst_case >
class SetRank
{
    using software_knob_geometry = margot::OperatingPointSegment< 1, margot::Data<int> >;
    using metrics_geometry = margot::OperatingPointSegment< 3, margot::Distribution<float> >;
    using MyOperatingPoint = margot::OperatingPoint< software_knob_geometry, metrics_geometry >;
    using avg_metric_0 = margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 0, 0>;
    using avg_metric_1 = margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 1, 0>;

    std::vector< MyOperatingPoint > op_list;
    margot::Asrtm<MyOperatingPoint> manager;
    margot::Goal< float, margot::ComparisonFunctions::GREATER > goal;

    int goal_priority;

  public:

    SetRank( const int number_of_ops )
    {
      // reserve the space in the Operating Point list
      op_list.reserve(number_of_ops);

      // create the Operating Point list
      for (int i = 0; i < number_of_ops; ++i)
      {
        op_list.push_back(
        {
          {i},
          {margot::Distribution<float>(number_of_ops - i, 0.1), margot::Distribution<float>(1, 0.1), margot::Distribution<float>(7, 0.1)}
        }
        );
      }

      // define the state of the Asrtm
      manager.create_new_state("optimization");
      manager.change_active_state("optimization");

      // define the goal
      if (worst_case)
      {
        goal.set(static_cast<float>(-1));
      }
      else
      {
        goal.set(static_cast<float>(number_of_ops + 1));
      }


      // add the operating points
      manager.add_operating_points(op_list);

      // set a constraint that invalidates all the ops
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(goal, 10);

    }


    inline void operator()( void )
    {
      manager.set_rank< margot::RankObjective::MAXIMIZE, margot::FieldComposer::LINEAR, avg_metric_0, avg_metric_1 >( 1.0f, 1.0f );
    }

};


#endif // MARGOT_TEST_SET_RANK_HDR
