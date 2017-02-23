/* core/asrtm
 * Copyright (C) 2017 Davide Gadioli
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <stdexcept>
#include <unistd.h>
#include <iostream>

#include <cxxtest/TestSuite.h>


#include <margot/config.hpp>
#include <margot/state.hpp>
#include <margot/time_monitor.hpp>
#include <margot/goal.hpp>



class StateTest : public CxxTest::TestSuite
{
		margot::operating_points_t points_two;
		margot::operating_points_t points_five;
		margot::operating_points_t points_seven;
		margot::monitor_t<float> my_monitor;
		margot::goal_t my_goal_greater;
		margot::goal_t my_goal_less;

	public:

		void setUp( void )
		{
			// initialize the two points list
			points_two =
			{
				{
					{ 1 },
					{ 1.0f, 1.0f, 7.0f}
				},
				{
					{ 2 },
					{1.0f, 2.0f, 6.0f}
				}
			};


			// initialize the five points list
			points_five =
			{
				{
					{3},
					{1.0f, 3.0f, 5.0f}
				},
				{
					{4},
					{1.0f, 4.0f, 4.0f}
				},
				{
					{5},
					{1.0f, 5.0f, 3.0f}
				},
				{
					{6},
					{1.0f, 6.0f, 2.0f}
				},
				{
					{7},
					{1.0f, 7.0f, 1.0f}
				},
			};

			// initialize the seven points list
			points_seven =
			{
				{
					{ 1 },
					{ 1.0f, 1.0f, 7.0f}
				},
				{
					{ 2 },
					{1.0f, 2.0f, 6.0f}
				},
				{
					{3},
					{1.0f, 3.0f, 5.0f}
				},
				{
					{4},
					{1.0f, 4.0f, 4.0f}
				},
				{
					{5},
					{1.0f, 5.0f, 3.0f}
				},
				{
					{6},
					{1.0f, 6.0f, 2.0f}
				},
				{
					{7},
					{1.0f, 7.0f, 1.0f}
				},
			};

			my_monitor = margot::monitor_t<float> {};
			my_goal_greater = margot::goal_t(my_monitor, margot::DataFunction::Average, margot::ComparisonFunction::Greater, static_cast<margot::time_monitor_t::value_type>(1));
			my_goal_less = margot::goal_t(my_monitor, margot::DataFunction::Average, margot::ComparisonFunction::Less, static_cast<margot::time_monitor_t::value_type>(1));
		}

		void test_state_creation_empty( void )
		{
			margot::state_t my_state;
		}















		/***********************************************
		 * TESTING the get best parameters function
		 ***********************************************/

		void test_get_best_op1( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);


			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 3);
		}

		void test_get_best_op2( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define an increasing rank on parameter
			my_state.define_linear_rank(margot::RankObjective::Maximize, margot::rank_parameter_t{0, 1.0f});


			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 7);
		}

		void test_get_best_op3( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define an decreasing rank on parameter
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});


			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 3);
		}

		void test_get_best_op4( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define an decreasing rank on parameter
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_metric_t{1, 1.0f});


			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 3);
		}

		void test_get_best_op5( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define an decreasing rank on parameter
			my_state.define_linear_rank(margot::RankObjective::Maximize, margot::rank_metric_t{1, 1.0f});


			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 7);
		}

		void test_get_best_op6( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});


			// add an useless rank parameter
			my_goal_greater.set(-4);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);


			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 3);
		}

		void test_get_best_op7( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});


			// add a middle effective rank
			my_goal_greater.set(4);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);


			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 5);
		}

		void test_get_best_op8( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});


			// add a too strict constraint
			my_goal_greater.set(20);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);


			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 7);
		}

		void test_get_best_op9( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});


			// add a useless constraint on top
			my_goal_greater.set(-4);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// add a useless constraint on bottom
			my_goal_less.set(7);
			my_state.add_metric_constraint(0, my_goal_less, 20);

			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 3);
		}

		void test_get_best_op10( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});


			// add a middle constraint on top
			my_goal_greater.set(4);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// add a useless constraint on bottom
			my_goal_less.set(7);
			my_state.add_metric_constraint(0, my_goal_less, 20);

			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 5);
		}

		void test_get_best_op11( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});


			// add a too strict constraint on top
			my_goal_greater.set(20);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// add a useless constraint on bottom
			my_goal_less.set(7);
			my_state.add_metric_constraint(0, my_goal_less, 20);

			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 7);
		}

		void test_get_best_op12( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});


			// add a useless constraint on top
			my_goal_greater.set(-5);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// add a midlle constraint on bottom
			my_goal_less.set(3);
			my_state.add_metric_constraint(2, my_goal_less, 20);

			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 6);
		}


		void test_get_best_op13( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});


			// add a middle constraint on top
			my_goal_greater.set(4);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// add a midlle constraint on bottom
			my_goal_less.set(3);
			my_state.add_metric_constraint(2, my_goal_less, 20);

			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 6);
		}

		void test_get_best_op14( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank on all the same values
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});


			// add a strict constraint on top on all the same values
			my_goal_greater.set(50);
			my_state.add_metric_constraint(0, my_goal_greater, 10);

			// add a strict constraint on bottom on all the same values
			my_goal_less.set(-5);
			my_state.add_metric_constraint(0, my_goal_less, 20);

			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 3);
		}

		void test_get_best_op15( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank on all the same values
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_metric_t{0, 1.0f});


			// add a strict constraint on top on all the same values
			my_goal_greater.set(50);
			my_state.add_metric_constraint(0, my_goal_greater, 10);

			// add a strict constraint on bottom on all the same values
			my_goal_less.set(-5);
			my_state.add_metric_constraint(0, my_goal_less, 20);

			// without rank should take the first OP
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 7); // the one that happens to be first in the block ops
		}


























		/***********************************************
		 * TESTING the OPs manipulation methods
		 ***********************************************/

		void test_remove_ops1( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_seven);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// the best one is the first
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 1);

			// add the OPs
			kb.remove_operating_points(points_two);
			my_state.remove_operating_points(points_two);

			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 3);
		}

		void test_remove_ops2( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_seven);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(1);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// the best one is the first
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 2);

			// add the OPs
			kb.remove_operating_points(points_two);
			my_state.remove_operating_points(points_two);

			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 3);
		}

		void test_remove_ops3( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_seven);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(1);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);


			// add a midlle constraint on bottom
			my_goal_less.set(3);
			my_state.add_metric_constraint(1, my_goal_less, 20);


			// the best one is the first
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 2);

			// add the OPs
			kb.remove_operating_points(points_two);
			my_state.remove_operating_points(points_two);

			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 3);
		}

		void test_add_ops1( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// the best one is the first
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 3);

			// add the OPs
			kb.add_operating_points(points_two);
			my_state.add_operating_points(points_two);

			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 1);
		}

		void test_add_ops2( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(1);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// the best one is the first
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 3);

			// add the OPs
			kb.add_operating_points(points_two);
			my_state.add_operating_points(points_two);

			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 2);
		}

		void test_add_ops3( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// define the rank
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(1);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);


			// add a midlle constraint on bottom
			my_goal_less.set(3);
			my_state.add_metric_constraint(1, my_goal_less, 20);


			// the best one is the first
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 3);

			// add the OPs
			kb.add_operating_points(points_two);
			my_state.add_operating_points(points_two);

			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 2);
		}

		void test_set_kb( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_two);
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(4);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// the best one is the second
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 2);

			// change the knowledge base
			margot::knowledge_base_t kb2;
			kb2.add_operating_points(points_five);

			// set the kb
			my_state.set_knowledge_base(kb2);

			// check out the new best op
			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 5);
		}

		void test_update1( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(-3);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// check the best op
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 3);

			// put the constraint in the middle
			my_goal_greater.set(5);
			my_state.update(best_op);


			// check the best op
			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 6);


			// put the constraint in the end
			my_goal_greater.set(20);
			my_state.update(best_op2);


			// check the best op
			const auto best_op3 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op3[0], 7);
		}

		void test_update2( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(20);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// check the best op
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 7);

			// put the constraint in the middle
			my_goal_greater.set(5);
			my_state.update(best_op);


			// check the best op
			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 6);


			// put the constraint in the end
			my_goal_greater.set(-3);
			my_state.update(best_op2);


			// check the best op
			const auto best_op3 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op3[0], 3);
		}

		void test_update3( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(1);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// check the best op
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 3);

			// put the constraint in the middle
			my_monitor.push(static_cast<float>(0.7));
			my_state.update(best_op);


			// check the best op
			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 5);


			// put the constraint in the end
			my_monitor.push(static_cast<float>(0.01));
			my_state.update(best_op2);

			// check the best op
			const auto best_op3 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op3[0], 7);
		}

		void test_update4( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(7);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// check the best op
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 7);

			// put the constraint in the middle
			my_monitor.push(static_cast<float>(14));
			my_state.update(best_op);


			// check the best op
			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 4);


			// put the constraint in the end
			my_monitor.push(static_cast<float>(400));
			my_state.update(best_op2);

			// check the best op
			const auto best_op3 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op3[0], 3);
		}

		void test_update5( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(7);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// check the best op
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 7);

			// put the constraint in the middle low with the values
			my_monitor.push(static_cast<float>(14));
			my_state.update(best_op);


			// check the best op
			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 4);

			// issue a clear on the monitors
			my_monitor.clear();

			// put the constraint in the middle low with the goal value
			my_goal_greater.set<float>(static_cast<float>(10));
			my_state.update(best_op2);

			// check the best op
			const auto best_op3 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op3[0], 6);
		}

















		/***********************************************
		 * TESTING the constraints manipulation methods
		 ***********************************************/

		void test_add_constraint1( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(3);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// check the best op
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 4);

			// add another constraint on top
			my_goal_less.set(4);
			my_state.add_metric_constraint(2, my_goal_less, 20);


			// check the best op
			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 5);
		}

		void test_add_constraint2( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(3);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// check the best op
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 4);

			// add another constraint on top
			my_goal_less.set(4);
			my_state.add_metric_constraint(2, my_goal_less, 5);


			// check the best op
			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 5);
		}

		void test_add_constraint3( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(100);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// check the best op
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 7);

			// add another constraint on top
			my_goal_less.set(4);
			my_state.add_metric_constraint(2, my_goal_less, 20);


			// check the best op
			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 7);

		}

		void test_add_constraint4( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(3);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// check the best op
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 4);

			// add another constraint on top
			my_goal_less.set(-5);
			my_state.add_metric_constraint(2, my_goal_less, 5);


			// check the best op
			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 7);
		}

		void test_remove_constraint1( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(3);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// check the best op
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 4);

			// add another constraint on top
			my_goal_less.set(4);
			my_state.add_metric_constraint(2, my_goal_less, 20);

			// check the best op
			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 5);

			// remove the constraint on top
			my_state.remove_constraint(10);

			// check the best op
			const auto best_op3 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op3[0], 5);
		}

		void test_remove_constraint2( void )
		{
			// set up
			margot::state_t my_state;
			margot::knowledge_base_t kb;
			kb.add_operating_points(points_five);
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a constraint on top
			my_goal_greater.set(3);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// check the best op
			const auto best_op = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op[0], 4);

			// add another constraint on top
			my_goal_less.set(4);
			my_state.add_metric_constraint(2, my_goal_less, 20);

			// check the best op
			const auto best_op2 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op2[0], 5);

			// remove the constraint on bottom
			my_state.remove_constraint(20);

			// check the best op
			const auto best_op3 = my_state.get_best_configuration();
			TS_ASSERT_EQUALS(best_op3[0], 4);
		}
};
