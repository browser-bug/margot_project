#ifndef MARGOT_TEST_ADD_CONSTRAINT_HDR
#define MARGOT_TEST_ADD_CONSTRAINT_HDR


#include "margot/asrtm.hpp"

#include "evaluator.hpp"



template< bool worst_case >
class AddConstraint
{
  using software_knob_geometry = margot::OperatingPointSegment< 1, margot::Data<int> >;
  using metrics_geometry = margot::OperatingPointSegment< 3, margot::Distribution<float> >;
  using MyOperatingPoint = margot::OperatingPoint< software_knob_geometry, metrics_geometry >;
  using avg_metric_0 = margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 0, 0>;

  std::vector< MyOperatingPoint > op_list;
  margot::Asrtm<MyOperatingPoint> manager;
  margot::Goal< float, margot::ComparisonFunctions::GREATER > goal;
  margot::Goal< float, margot::ComparisonFunctions::GREATER > goal1;

  int goal_priority;

public:

  AddConstraint( const int number_of_ops )
  {
    // reserve the space in the Operating Point list
    op_list.reserve(number_of_ops);

    // create the Operating Point list
    for(int i = 0; i < number_of_ops; ++i)
    {
      op_list.push_back(
        {
          {i},
          {margot::Distribution<float>(i, 0.1), margot::Distribution<float>(1, 0.1), margot::Distribution<float>(7, 0.1)}
        }
      );
    }

    // define the state of the Asrtm
    manager.create_new_state("optimization");
    manager.change_active_state("optimization");

    // define the goal
    goal.set(static_cast<float>(number_of_ops+1));
    goal1.set(static_cast<float>(number_of_ops+1));

    // add the operating points
    manager.add_operating_points(op_list);

    // set a constraint that invalidates all the ops
    manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(goal1, 10);

    // set the priority for the new constraint
    if (worst_case)
    {
      goal_priority = 5;
    }
    else
    {
      goal_priority = 15;
    }

  }


  inline void operator()( void )
  {
    manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(goal, goal_priority);
  }

};


#endif // MARGOT_TEST_ADD_CONSTRAINT_HDR
