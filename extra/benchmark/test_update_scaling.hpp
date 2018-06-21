#ifndef MARGOT_TEST_UPDATE_SCALING_HDR
#define MARGOT_TEST_UPDATE_SCALING_HDR


#include "margot/asrtm.hpp"

#include "evaluator.hpp"



template< int number_of_constraint >
class UpdateScaling
{
    using software_knob_geometry = margot::OperatingPointSegment< 1, margot::Data<int> >;
    using metrics_geometry = margot::OperatingPointSegment< 3, margot::Distribution<float> >;
    using MyOperatingPoint = margot::OperatingPoint< software_knob_geometry, metrics_geometry >;
    using avg_metric_0 = margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 0, 0>;
    using avg_metric_1 = margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 1, 0>;

    std::vector< MyOperatingPoint > op_list;
    margot::Asrtm<MyOperatingPoint> manager;
    margot::Goal< float, margot::ComparisonFunctions::GREATER_OR_EQUAL > control_goal;
    margot::Goal< float, margot::ComparisonFunctions::GREATER_OR_EQUAL > unsatisfiable_goal;

  public:

    UpdateScaling( const int number_of_ops )
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
      unsatisfiable_goal.set(static_cast<float>(2));
      control_goal.set(static_cast<float>(-1));


      // add the operating points
      manager.add_operating_points(op_list);

      // set a constraint that invalidates all the ops
      manager.add_constraint< margot::OperatingPointSegments::METRICS, 1, 0 >(control_goal, 0);

      for ( int i = 0; i < number_of_constraint; ++i)
      {
        manager.add_constraint< margot::OperatingPointSegments::METRICS, 1, 0 >(unsatisfiable_goal, i + 1);
      }


      // find the best op
      manager.find_best_configuration();
      manager.configuration_applied();

      // change the value of the goal
      control_goal.set(static_cast<float>(2));

    }


    inline void operator()( void )
    {
      manager.find_best_configuration();
    }

};


#endif // MARGOT_TEST_UPDATE_SCALING_HDR
