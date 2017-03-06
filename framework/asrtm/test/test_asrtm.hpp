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
#include <iostream>

#include <cxxtest/TestSuite.h>


#include <margot/config.hpp>
#include <margot/asrtm.hpp>
#include <margot/monitor.hpp>
#include <margot/goal.hpp>




class AsrtmTest : public CxxTest::TestSuite
{

		margot::operating_points_t default_points;

	public:


		void setUp( void )
		{
			default_points =
			{
				{{1}, {1, 1, 5}},
				{{2}, {1, 2, 4}},
				{{3}, {1, 3, 3}},
				{{4}, {1, 4, 2}},
				{{5}, {1, 5, 1}},
			};
		}

		void tearDown( void )
		{
			default_points.clear();
		}


		void test_creation( void )
		{
			margot::asrtm_t manager;

			margot::goal_t my_goal;
			TS_ASSERT_THROWS_NOTHING(my_goal = manager.create_static_goal_metric(0, margot::ComparisonFunction::Greater, 5));

			// test the initialized state
			TS_ASSERT_THROWS_NOTHING(manager.add_metric_constraint(my_goal, 0, 10));
			TS_ASSERT_THROWS_NOTHING(manager.add_parameter_constraint(my_goal, 0, 10));
			margot::configuration_t my_conf;
			TS_ASSERT_THROWS_NOTHING(my_conf = manager.get_best_configuration());
			TS_ASSERT(my_conf.empty());
			TS_ASSERT_THROWS_NOTHING(manager.find_best_operating_point());
			TS_ASSERT_THROWS_NOTHING(manager.define_geometric_rank(margot::RankObjective::Maximize, margot::rank_metric_t{0, 1.0f}));
			TS_ASSERT_THROWS_NOTHING(manager.define_linear_rank(margot::RankObjective::Maximize, margot::rank_metric_t{0, 1.0f}));
		}


		void test_post_creation( void )
		{
			// create the manager
			margot::asrtm_t manager;

			// create a static_goal
			margot::goal_t my_goal = manager.create_static_goal_metric(0, margot::ComparisonFunction::Greater, 5);

			// add operating points
			margot::operating_points_t points =
			{
				{{2, 4, 3}, {6, 4, 5}}
			};
			manager.add_operating_points(points);

			// check if the error is ok
			TS_ASSERT(my_goal.check());
			TS_ASSERT_EQUALS(my_goal.absolute_error(), 0);


			// check the observable value
			margot::metric_t value;
			TS_ASSERT(!my_goal.observed_value(value));
			TS_ASSERT_EQUALS(value, 6);
		}


		void test_post_creation2( void )
		{
			// create the manager
			margot::asrtm_t manager;

			// create a static_goal
			margot::goal_t my_goal = manager.create_static_goal_parameter(0, margot::ComparisonFunction::Greater, 1);

			// add operating points
			margot::operating_points_t points =
			{
				{{2, 4, 3}, {6, 4, 5}}
			};
			manager.add_operating_points(points);

			// check if the error is ok
			TS_ASSERT(my_goal.check());
			TS_ASSERT_EQUALS(my_goal.absolute_error(), 0);


			// check the observable value
			margot::metric_t value;
			TS_ASSERT(!my_goal.observed_value(value));
			TS_ASSERT_EQUALS(value, 2);
		}


		void test_constraint_management( void )
		{
			// create the manager
			margot::asrtm_t manager;

			// add the operating points
			manager.add_operating_points(default_points);

			// create the goals
			margot::goal_t g1 = manager.create_static_goal_metric(1, margot::ComparisonFunction::Greater, 3);
			margot::goal_t g2 = manager.create_static_goal_metric(1, margot::ComparisonFunction::Greater, 4);

			// set the rank
			manager.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.9f});

			// check if it is correct the starting point
			manager.find_best_operating_point();
			const auto number = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number, 1);

			// add the first goal
			manager.add_metric_constraint(g1, 1, 10);

			// retrieve the best parameter
			manager.find_best_operating_point();
			const auto number2 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number2, 4);

			// add the second goal
			manager.add_metric_constraint(g2, 1, 20);


			// retrieve the best parameter
			manager.find_best_operating_point();
			const auto number3 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number3, 5);


			// remove the outher parameter
			manager.remove_constraint(20);

			// retrieve the best parameter
			manager.find_best_operating_point();
			const auto number4 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number4, 4);


			// remove the outher parameter
			manager.remove_constraint(10);

			// retrieve the best parameter
			manager.find_best_operating_point();
			const auto number5 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number5, 1);
		}



		void test_constraint_management2( void )
		{
			// create the manager
			margot::asrtm_t manager;

			// add the operating points
			manager.add_operating_points(default_points);

			// create the goals
			margot::goal_t g1 = manager.create_static_goal_parameter(0, margot::ComparisonFunction::Greater, 3);
			margot::goal_t g2 = manager.create_static_goal_parameter(0, margot::ComparisonFunction::Greater, 4);

			// set the rank
			manager.define_geometric_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.9f});

			// check if it is correct the starting point
			manager.find_best_operating_point();
			const auto number = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number, 1);

			// add the first goal
			manager.add_parameter_constraint(g1, 0, 10);

			// retrieve the best parameter
			manager.find_best_operating_point();
			const auto number2 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number2, 4);

			// add the second goal
			manager.add_parameter_constraint(g2, 0, 20);


			// retrieve the best parameter
			manager.find_best_operating_point();
			const auto number3 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number3, 5);


			// remove the outher parameter
			manager.remove_constraint(20);

			// retrieve the best parameter
			manager.find_best_operating_point();
			const auto number4 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number4, 4);


			// remove the outher parameter
			manager.remove_constraint(10);

			// retrieve the best parameter
			manager.find_best_operating_point();
			const auto number5 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number5, 1);
		}




		void test_state_management( void )
		{
			// declare the manager
			margot::asrtm_t manager;
			manager.add_operating_points(default_points);
			manager.define_linear_rank(margot::RankObjective::Maximize, margot::rank_metric_t{1, 0.5f});
			manager.configuration_applied();

			// retrieve the best parameter
			manager.find_best_operating_point();
			const auto number1 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number1, 5);

			// try to remove the active state
			TS_ASSERT_THROWS(manager.remove_state("default"), std::runtime_error);

			// try ro switch to a non existent state
			TS_ASSERT_THROWS(manager.change_active_state("pippo"), std::runtime_error);


			// create & switch to a new state
			manager.add_state("pippo");
			manager.change_active_state("pippo");

			// define a rank to the opposite direction
			manager.define_geometric_rank(margot::RankObjective::Minimize, margot::rank_metric_t{1, 2});

			// retrieve the best parameter
			manager.find_best_operating_point();
			const auto number2 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number2, 1);


			// switch back to the previous state
			manager.change_active_state("default");


			// retrieve the best parameter
			manager.find_best_operating_point();
			const auto number3 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number3, 5);

			// delete the pippo state and tries to switch back to the previous state
			manager.remove_state("pippo");
			TS_ASSERT_THROWS(manager.change_active_state("pippo"), std::runtime_error);
		}



		void test_operating_point_removal( void )
		{
			// create a manager which selects the last op in the list
			margot::asrtm_t manager;
			manager.add_operating_points(default_points);
			manager.define_linear_rank(margot::RankObjective::Maximize, margot::rank_metric_t{1, 0.5f});
			manager.configuration_applied();

			// retrieve the best parameter
			manager.update();
			manager.find_best_operating_point();
			const auto number = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number, 5);


			// remove the used configuration
			margot::operating_points_t op_to_remove = {
				{{5},{1,5,1}}
			};
			manager.remove_operating_points(op_to_remove);

			// get a mew configuration
			manager.update();
			manager.find_best_operating_point();
			const auto number2 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number2, 4);


			// add again the best configuration
			margot::operating_points_t op_to_add = {
				{{5},{1,5,1}}
			};
			manager.add_operating_points(op_to_add);


			// get a mew configuration
			manager.update();
			manager.find_best_operating_point();
			const auto number3 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number3, 5);
		}

		void test_operating_point_removal_with_goal( void )
		{
			// create a manager which selects the last op in the list
			margot::asrtm_t manager;
			manager.add_operating_points(default_points);
			manager.define_linear_rank(margot::RankObjective::Maximize, margot::rank_metric_t{1, 0.5f});
			manager.configuration_applied();

			// create a static goal
			auto goal = manager.create_static_goal_metric(0, margot::ComparisonFunction::LessOrEqual, 100);
			manager.add_metric_constraint(goal, 0, 12);

			// retrieve the best parameter
			manager.update();
			manager.find_best_operating_point();
			const auto number = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number, 5);


			// remove the used configuration
			margot::operating_points_t op_to_remove = {
				{{5},{1,5,1}}
			};
			manager.remove_operating_points(op_to_remove);

			// get a mew configuration
			manager.update();
			manager.find_best_operating_point();
			const auto number2 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number2, 4);


			// add again the best configuration
			margot::operating_points_t op_to_add = {
				{{5},{1,5,1}}
			};
			manager.add_operating_points(op_to_add);


			// get a mew configuration
			manager.update();
			manager.find_best_operating_point();
			const auto number3 = manager.get_best_configuration()[0];
			manager.configuration_applied();
			TS_ASSERT_EQUALS(number3, 5);
		}

};
