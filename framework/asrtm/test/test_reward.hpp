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



class RewardTest : public CxxTest::TestSuite
{
		margot::operating_points_t points_five;
		margot::monitor_t<float> my_monitor;
		margot::goal_t my_goal_greater;
		margot::goal_t my_goal_less;
		margot::goal_t my_goal_greater2;

	public:

		void setUp( void )
		{
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

			my_monitor = margot::monitor_t<float> {};
			my_goal_greater = margot::goal_t(my_monitor, margot::DataFunction::Average, margot::ComparisonFunction::Greater, static_cast<margot::time_monitor_t::value_type>(1));
			my_goal_less = margot::goal_t(my_monitor, margot::DataFunction::Average, margot::ComparisonFunction::Less, static_cast<margot::time_monitor_t::value_type>(1));
			my_goal_greater2 = margot::goal_t(my_monitor, margot::DataFunction::Average, margot::ComparisonFunction::Greater, static_cast<margot::time_monitor_t::value_type>(1));

		}


		/***********************************************
		 * TESTING the reward related methods
		 ***********************************************/

		void test_reward_changing_structure_up( void )
		{
			// define the state and the knowledge base
			margot::state_t my_state;
			margot::knowledge_base_t kb;

			// insert one initial Operating Point
			kb.add_operating_points(
			{
				{
					{2}, {1, 3, 5}
				}
			}
			);

			// add it to the state
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Maximize, margot::rank_parameter_t{0, 1.0f});
			auto best_conf = my_state.get_best_configuration();
			const float reward0 = my_state.get_reward(best_conf);

			// add another Operating Point
			auto conf_to_be_added = margot::operating_points_t{{{3}, {1, 4, 4}}};
			kb.add_operating_points(conf_to_be_added);
			my_state.add_operating_points(conf_to_be_added);
			best_conf = my_state.get_best_configuration();
			const float reward1 = my_state.get_reward(best_conf);

			// add another Operating Point
			conf_to_be_added = margot::operating_points_t{{{4}, {1, 5, 3}}};
			kb.add_operating_points(conf_to_be_added);
			my_state.add_operating_points(conf_to_be_added);
			best_conf = my_state.get_best_configuration();
			const float reward2 = my_state.get_reward(best_conf);

			// add another Operating Point
			conf_to_be_added = margot::operating_points_t{{{6}, {1, 7, 1}}};
			kb.add_operating_points(conf_to_be_added);
			my_state.add_operating_points(conf_to_be_added);
			best_conf = my_state.get_best_configuration();
			const float reward3 = my_state.get_reward(best_conf);

			// add the last Operating Point
			conf_to_be_added = margot::operating_points_t{{{5}, {1, 6, 2}}};
			kb.add_operating_points(conf_to_be_added);
			my_state.add_operating_points(conf_to_be_added);
			best_conf = my_state.get_best_configuration();
			const float reward4 = my_state.get_reward(best_conf);

			// check the monotonicity of the reward
			TS_ASSERT_DELTA(reward0, reward1, 0.001);
			TS_ASSERT_DELTA(reward1, reward2, 0.001);
			TS_ASSERT_DELTA(reward2, reward3, 0.001);
			TS_ASSERT_DELTA(reward3, reward4, 0.001);
		}


		void test_reward_changing_structure_down( void )
		{
			// define the state and the knowledge base
			margot::state_t my_state;
			margot::knowledge_base_t kb;

			// insert one initial Operating Point
			kb.add_operating_points(points_five);

			// add it to the state
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Maximize, margot::rank_parameter_t{0, 1.0f});
			auto best_conf = my_state.get_best_configuration();
			const float reward0 = my_state.get_reward(best_conf);

			// remove an Operating Point
			auto conf_to_be_removed = margot::operating_points_t{{{7}, {1.0f, 7.0f, 1.0f}}};
			kb.remove_operating_points(conf_to_be_removed);
			my_state.remove_operating_points(conf_to_be_removed);
			best_conf = my_state.get_best_configuration();
			const float reward1 = my_state.get_reward(best_conf);


			// remove an Operating Point
			conf_to_be_removed = margot::operating_points_t{{{5}, {1.0f, 5.0f, 3.0f}}};
			kb.remove_operating_points(conf_to_be_removed);
			my_state.remove_operating_points(conf_to_be_removed);
			best_conf = my_state.get_best_configuration();
			const float reward2 = my_state.get_reward(best_conf);


			// remove an Operating Point
			conf_to_be_removed = margot::operating_points_t{{{6}, {1.0f, 6.0f, 2.0f}}};
			kb.remove_operating_points(conf_to_be_removed);
			my_state.remove_operating_points(conf_to_be_removed);
			best_conf = my_state.get_best_configuration();
			const float reward3 = my_state.get_reward(best_conf);


			// remove an Operating Point
			conf_to_be_removed = margot::operating_points_t{{{4}, {1.0f, 4.0f, 4.0f}}};
			kb.remove_operating_points(conf_to_be_removed);
			my_state.remove_operating_points(conf_to_be_removed);
			best_conf = my_state.get_best_configuration();
			const float reward4 = my_state.get_reward(best_conf);


			// check the monotonicity of the reward
			TS_ASSERT_DELTA(reward0, reward1, 0.001);
			TS_ASSERT_DELTA(reward1, reward2, 0.001);
			TS_ASSERT_DELTA(reward2, reward3, 0.001);
			TS_ASSERT_DELTA(reward3, reward4, 0.001);
		}

		void test_reward_stress( void )
		{
			// define the state and the knowledge base
			margot::state_t my_state;
			margot::knowledge_base_t kb;

			// insert one initial Operating Point
			kb.add_operating_points(points_five);

			// add it to the state
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a too strict constraint
			my_goal_greater.set(2);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// get the initial reward
			auto best_conf = my_state.get_best_configuration();
			const float reward0 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(3);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward1 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(4);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward2 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(5);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward3 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(6);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward4 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(20);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward5 = my_state.get_reward(best_conf);

			// check the monotonicity of the reward
			TS_ASSERT_LESS_THAN(reward1, reward0);
			TS_ASSERT_LESS_THAN(reward2, reward1);
			TS_ASSERT_LESS_THAN(reward3, reward2);
			TS_ASSERT_LESS_THAN(reward4, reward3);
			TS_ASSERT_LESS_THAN(reward5, reward4);
			//TS_ASSERT_DELTA(reward4, reward5, 0.001);
		}

		void test_reward_stress_inversed( void )
		{
			// define the state and the knowledge base
			margot::state_t my_state;
			margot::knowledge_base_t kb;

			// insert one initial Operating Point
			kb.add_operating_points(points_five);

			// add it to the state
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add a too strict constraint
			my_goal_greater.set(20);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// get the initial reward
			auto best_conf = my_state.get_best_configuration();
			const float reward0 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(6);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward1 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(5);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward2 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(4);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward3 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(3);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward4 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(2);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward5 = my_state.get_reward(best_conf);

			// check the monotonicity of the reward
			//TS_ASSERT_DELTA(reward0, reward1, 0.001);
			TS_ASSERT_LESS_THAN(reward0, reward1);
			TS_ASSERT_LESS_THAN(reward1, reward2);
			TS_ASSERT_LESS_THAN(reward2, reward3);
			TS_ASSERT_LESS_THAN(reward3, reward4);
			TS_ASSERT_LESS_THAN(reward4, reward5);
		}

		void test_reward_useless_proof( void )
		{
			// define the state and the knowledge base
			margot::state_t my_state;
			margot::knowledge_base_t kb;

			// insert one initial Operating Point
			kb.add_operating_points(points_five);

			// add it to the state
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Maximize, margot::rank_parameter_t{0, 1.0f});

			// add a too strict constraint
			my_goal_greater.set(20);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);

			// get the initial reward
			auto best_conf = my_state.get_best_configuration();
			const float reward0 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(6);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward1 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(5);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward2 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(4);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward3 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(3);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward4 = my_state.get_reward(best_conf);

			// relax the constraint
			my_goal_greater.set(2);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward5 = my_state.get_reward(best_conf);

			// check the monotonicity of the reward
			TS_ASSERT_LESS_THAN(reward0, reward1);
			TS_ASSERT_DELTA(reward1, reward2, 0.001);
			TS_ASSERT_DELTA(reward2, reward3, 0.001);
			TS_ASSERT_DELTA(reward3, reward4, 0.001);
			TS_ASSERT_DELTA(reward4, reward5, 0.001);
		}


		void test_reward_priority_matter_extreme_3( void )
		{
			// define the state and the knowledge base
			margot::state_t my_state;
			margot::knowledge_base_t kb;

			// insert one initial Operating Point
			kb.add_operating_points(points_five);

			// add it to the state
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add three constraint wich lead to two valid points
			my_goal_greater.set(5);
			my_goal_less.set(8);
			my_goal_greater2.set(0);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);
			my_state.add_metric_constraint(1, my_goal_less, 20);
			my_state.add_metric_constraint(2, my_goal_greater2, 30);

			// get the initial reward
			auto best_conf = my_state.get_best_configuration();
			const float reward0 = my_state.get_reward(best_conf);

			// invalidate the first constraint only
			my_goal_greater.set(20);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward1 = my_state.get_reward(best_conf);

			// invalidate also the second constraint
			my_goal_less.set(0);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward2 = my_state.get_reward(best_conf);

			// invalidate all the constraints
			my_goal_greater2.set(20);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward3 = my_state.get_reward(best_conf);

			// check the reward
			TS_ASSERT_LESS_THAN(reward3, reward2);
			TS_ASSERT_LESS_THAN(reward2, reward1);
			TS_ASSERT_LESS_THAN(reward1, reward0);
		}

		void test_reward_priority_matter_almost_3( void )
		{
			// define the state and the knowledge base
			margot::state_t my_state;
			margot::knowledge_base_t kb;

			// insert one initial Operating Point
			kb.add_operating_points(points_five);

			// add it to the state
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add three constraint wich lead to two valid points
			my_goal_greater.set(5);
			my_goal_less.set(8);
			my_goal_greater2.set(0);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);
			my_state.add_metric_constraint(1, my_goal_less, 20);
			my_state.add_metric_constraint(2, my_goal_greater2, 30);

			// get the initial reward
			auto best_conf = my_state.get_best_configuration();
			const float reward0 = my_state.get_reward(best_conf);

			// invalidate the first constraint only
			my_goal_greater.set(20.0f);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward1 = my_state.get_reward(best_conf);

			// invalidate also the second constraint
			my_goal_less.set(6.9f);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward2 = my_state.get_reward(best_conf);

			// invalidate all the constraints
			my_goal_greater2.set(1.1f);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward3 = my_state.get_reward(best_conf);

			// check the reward
			TS_ASSERT_LESS_THAN(reward3, reward2);
			TS_ASSERT_LESS_THAN(reward2, reward1);
			TS_ASSERT_LESS_THAN(reward1, reward0);
		}

		void test_reward_priority_matter_comparison_3( void )
		{
			// define the state and the knowledge base
			margot::state_t my_state;
			margot::knowledge_base_t kb;

			// insert one initial Operating Point
			kb.add_operating_points(points_five);

			// add it to the state
			my_state.set_knowledge_base(kb);

			// set the rank definition
			my_state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// add three constraint wich lead to two valid points
			my_goal_greater.set(5);
			my_goal_less.set(8);
			my_goal_greater2.set(0);
			my_state.add_parameter_constraint(0, my_goal_greater, 10);
			my_state.add_metric_constraint(1, my_goal_less, 20);
			my_state.add_metric_constraint(2, my_goal_greater2, 30);

			// get the initial reward
			auto best_conf = my_state.get_best_configuration();
			const float reward0 = my_state.get_reward(best_conf);

			// invalidate the first constraint badly
			my_goal_greater.set(20.0f);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward1 = my_state.get_reward(best_conf);

			// invalidate the second constraint badly
			my_goal_less.set(0.0f);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward2 = my_state.get_reward(best_conf);

			// invalidate the second constraint barely
			my_goal_less.set(6.9f);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward3 = my_state.get_reward(best_conf);

			// invalidate the third constraint badly
			my_goal_greater2.set(20.0f);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward4 = my_state.get_reward(best_conf);

			// invalidate the third constraint barely
			my_goal_greater2.set(1.1f);
			my_state.update(best_conf);
			best_conf = my_state.get_best_configuration();
			const float reward5 = my_state.get_reward(best_conf);

			// check the rewards between the same level
			TS_ASSERT_LESS_THAN(reward4, reward5);
			TS_ASSERT_LESS_THAN(reward2, reward3);

			// check the full stack backward
			TS_ASSERT_LESS_THAN(reward5, reward3);
			TS_ASSERT_LESS_THAN(reward3, reward1);
			TS_ASSERT_LESS_THAN(reward1, reward0);

			// check between min and max
			TS_ASSERT_LESS_THAN(reward4, reward3);
			TS_ASSERT_LESS_THAN(reward2, reward1);
		}




};
