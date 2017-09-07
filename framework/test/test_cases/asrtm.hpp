#include <vector>

#include <cxxtest/TestSuite.h>

#include <margot/asrtm.hpp>
#include <margot/goal.hpp>

class Asrtm : public CxxTest::TestSuite
{

    using software_knob_geometry = margot::OperatingPointSegment< 1, margot::Data<int> >;
    using metrics_geometry = margot::OperatingPointSegment< 3, margot::Distribution<float> >;
    using MyOperatingPoint = margot::OperatingPoint< software_knob_geometry, metrics_geometry >;

    std::vector< MyOperatingPoint > op_list_2;
    std::vector< MyOperatingPoint > op_list_5;
    std::vector< MyOperatingPoint > op_list_7;


    margot::Goal< float, margot::ComparisonFunctions::GREATER > greater_goal;
    margot::Goal< float, margot::ComparisonFunctions::GREATER_OR_EQUAL > greater_or_equal;
    margot::Goal< float, margot::ComparisonFunctions::LESS > less_goal;

    // definition for rank
    using avg_knob_0 = margot::OPField<margot::OperatingPointSegments::SOFTWARE_KNOBS, margot::BoundType::LOWER, 0, 0>;
    using avg_metric_0 = margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 0, 0>;
    using avg_metric_1 = margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 1, 0>;
    using avg_metric_2 = margot::OPField<margot::OperatingPointSegments::METRICS, margot::BoundType::LOWER, 2, 0>;

  public:


    void setUp( void )
    {

      op_list_2 = std::vector< MyOperatingPoint >(
      {
        {
          {1},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(1, 0.1), margot::Distribution<float>(7, 0.1)}
        },
        {
          {2},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(2, 0.1), margot::Distribution<float>(6, 0.1)}
        }
      });


      op_list_5 = std::vector< MyOperatingPoint >(
      {
        {
          {3},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(3, 0.1), margot::Distribution<float>(5, 0.1)}
        },
        {
          {4},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(4, 0.1), margot::Distribution<float>(4, 0.1)}
        },
        {
          {5},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(5, 0.1), margot::Distribution<float>(3, 0.1)}
        },
        {
          {6},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(6, 0.1), margot::Distribution<float>(2, 0.1)}
        },
        {
          {7},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(7, 0.1), margot::Distribution<float>(1, 0.1)}
        }
      });


      op_list_7 = std::vector< MyOperatingPoint >(
      {
        {
          {1},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(1, 0.1), margot::Distribution<float>(7, 0.1)}
        },
        {
          {2},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(2, 0.1), margot::Distribution<float>(6, 0.1)}
        },
        {
          {3},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(3, 0.1), margot::Distribution<float>(5, 0.1)}
        },
        {
          {4},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(4, 0.1), margot::Distribution<float>(4, 0.1)}
        },
        {
          {5},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(5, 0.1), margot::Distribution<float>(3, 0.1)}
        },
        {
          {6},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(6, 0.1), margot::Distribution<float>(2, 0.1)}
        },
        {
          {7},
          {margot::Distribution<float>(1, 0.1), margot::Distribution<float>(7, 0.1), margot::Distribution<float>(1, 0.1)}
        }
      });




    }

    void tearDown( void )
    {
      op_list_2.clear();
    }


    void test_creation_empty( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.find_best_configuration();
      //manager.get_best_configuration();
    }

    void test_utility_get( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");
      manager.find_best_configuration();
      manager.get_best_configuration();
      manager.configuration_applied();

      TS_ASSERT_EQUALS((manager.get_mean<margot::OperatingPointSegments::SOFTWARE_KNOBS, 0>()), 3)
      TS_ASSERT_EQUALS((manager.get_mean<margot::OperatingPointSegments::METRICS, 0>()), 1)
    }












    /***********************************************
     * TESTING the decisional algorithm
     ***********************************************/


    void test_get_best_op1( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);
    }

    void test_get_best_op2( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MAXIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
    }

    void test_get_best_op3( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);
    }

    void test_get_best_op4( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_metric_1 >( 1.0f );


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);
    }

    void test_get_best_op5( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MAXIMIZE, margot::FieldComposer::SIMPLE, avg_metric_1 >( 1.0f );


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
    }

    void test_get_best_op6( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(-4);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);
    }

    void test_get_best_op7( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(4);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 5);
    }

    void test_get_best_op8( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(20);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
    }

    void test_get_best_op9( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(-4);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      less_goal.set(7);
      manager.add_constraint< margot::OperatingPointSegments::METRICS, 0, 0 >(less_goal, 20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);
    }

    void test_get_best_op10( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(4);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      less_goal.set(7);
      manager.add_constraint< margot::OperatingPointSegments::METRICS, 0, 0 >(less_goal, 20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 5);
    }

    void test_get_best_op11( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(20);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      less_goal.set(7);
      manager.add_constraint< margot::OperatingPointSegments::METRICS, 0, 0 >(less_goal, 20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
    }

    void test_get_best_op12( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(-5);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      less_goal.set(3);
      manager.add_constraint< margot::OperatingPointSegments::METRICS, 2, 0 >(less_goal, 20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 6);
    }

    void test_get_best_op13( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(4);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      less_goal.set(3);
      manager.add_constraint< margot::OperatingPointSegments::METRICS, 2, 0 >(less_goal, 20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 6);
    }

    void test_get_best_op14( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(50);
      manager.add_constraint< margot::OperatingPointSegments::METRICS, 0, 0 >(greater_goal, 10);

      less_goal.set(-5);
      manager.add_constraint< margot::OperatingPointSegments::METRICS, 0, 0 >(less_goal, 20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);
    }

    void test_get_best_op15( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MAXIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(50);
      manager.add_constraint< margot::OperatingPointSegments::METRICS, 0, 0 >(greater_goal, 10);

      less_goal.set(-5);
      manager.add_constraint< margot::OperatingPointSegments::METRICS, 0, 0 >(less_goal, 20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
    }













    /***********************************************
    * TESTING the OPs manipulation methods
    ***********************************************/

    void test_remove_ops1( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_7);
      manager.create_new_state("default");
      manager.change_active_state("default");


      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 1);

      manager.remove_operating_points(op_list_2);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);

    }

    void test_remove_ops2( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_7);
      manager.create_new_state("default");
      manager.change_active_state("default");


      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(1);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 2);

      manager.remove_operating_points(op_list_2);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);

    }

    void test_remove_ops3( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_7);
      manager.create_new_state("default");
      manager.change_active_state("default");


      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(1);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      less_goal.set(3);
      manager.add_constraint< margot::OperatingPointSegments::METRICS, 1, 0 >(less_goal, 20);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 2);

      manager.remove_operating_points(op_list_2);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);

    }

    void test_add_ops1( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");


      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);

      manager.add_operating_points(op_list_2);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 1);

    }

    void test_add_ops2( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");


      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(1);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);

      manager.add_operating_points(op_list_2);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 2);

    }

    void test_add_ops3( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");


      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(1);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      less_goal.set(3);
      manager.add_constraint< margot::OperatingPointSegments::METRICS, 1, 0 >(less_goal, 20);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);

      manager.add_operating_points(op_list_2);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 2);

    }




    /***********************************************
    * TESTING the update method
    ***********************************************/

    void test_update1a( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(-3);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);

      greater_goal.set(5);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 6);

      greater_goal.set(20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
    }

    void test_update2a( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(20);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
      greater_goal.set(5);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 6);

      greater_goal.set(-3);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);
    }

    void test_update1b( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_or_equal.set(-3);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_or_equal, 10);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);

      greater_or_equal.set(5);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 5);

      greater_or_equal.set(20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
    }

    void test_update2b( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_or_equal.set(20);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_or_equal, 10);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
      greater_or_equal.set(5);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 5);

      greater_or_equal.set(-3);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);
    }

    void test_update3( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(1);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);
      manager.configuration_applied();

      margot::Monitor< float > monitor;
      manager.add_runtime_knowledge<margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 1>(monitor);
      monitor.push(0.7f);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 5);
      manager.configuration_applied();

      monitor.push(0.01f);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
    }

    void test_update4( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(7);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
      manager.configuration_applied();

      margot::Monitor< float > monitor;
      manager.add_runtime_knowledge<margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 1>(monitor);
      monitor.push(14);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 4);
      manager.configuration_applied();

      monitor.push(400);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);
    }

    void test_update5( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(7);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
      manager.configuration_applied();

      margot::Monitor< float > monitor;
      manager.add_runtime_knowledge<margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 1>(monitor);
      monitor.push(14);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 4);
      manager.configuration_applied();

      monitor.clear();

      greater_goal.set(10);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 6);
    }








    /***********************************************
    * TESTING the add/remove constraint methods
    ***********************************************/

    void test_add_constraint1( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(3);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 4);
      manager.configuration_applied();

      less_goal.set(4);
      manager.add_constraint<margot::OperatingPointSegments::METRICS, 2, 0>(less_goal, 20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 5);
      manager.configuration_applied();
    }

    void test_add_constraint2( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(3);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 4);
      manager.configuration_applied();

      less_goal.set(4);
      manager.add_constraint<margot::OperatingPointSegments::METRICS, 2, 0>(less_goal, 5);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 5);
      manager.configuration_applied();
    }

    void test_add_constraint3( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(100);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
      manager.configuration_applied();

      less_goal.set(4);
      manager.add_constraint<margot::OperatingPointSegments::METRICS, 2, 0>(less_goal, 20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
      manager.configuration_applied();
    }

    void test_add_constraint4( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(100);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
      manager.configuration_applied();

      less_goal.set(4);
      manager.add_constraint<margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0>(less_goal, 5);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 3);
      manager.configuration_applied();
    }

    void test_add_constraint5( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(3);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 4);
      manager.configuration_applied();

      less_goal.set(-5);
      manager.add_constraint<margot::OperatingPointSegments::METRICS, 2, 0>(less_goal, 5);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 7);
      manager.configuration_applied();
    }

    void test_remove_constraint1( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(3);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 4);
      manager.configuration_applied();

      less_goal.set(4);
      manager.add_constraint<margot::OperatingPointSegments::METRICS, 2, 0>(less_goal, 20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 5);
      manager.configuration_applied();


      manager.remove_constraint(10);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 5);
      manager.configuration_applied();
    }

    void test_remove_constraint2( void )
    {
      margot::Asrtm<MyOperatingPoint> manager;
      manager.add_operating_points(op_list_5);
      manager.create_new_state("default");
      manager.change_active_state("default");

      manager.set_rank< margot::RankObjective::MINIMIZE, margot::FieldComposer::SIMPLE, avg_knob_0 >( 1.0f );

      greater_goal.set(3);
      manager.add_constraint< margot::OperatingPointSegments::SOFTWARE_KNOBS, 0, 0 >(greater_goal, 10);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 4);
      manager.configuration_applied();

      less_goal.set(4);
      manager.add_constraint<margot::OperatingPointSegments::METRICS, 2, 0>(less_goal, 20);


      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 5);
      manager.configuration_applied();


      manager.remove_constraint(20);

      manager.find_best_configuration();
      TS_ASSERT_EQUALS(manager.get_best_configuration().get_mean<0>(), 4);
      manager.configuration_applied();
    }

};
