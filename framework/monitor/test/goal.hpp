/* core/monitor
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

#include <functional>
#include <cmath>
#include <iostream>

#include <cxxtest/TestSuite.h>


#include <margot/monitor.hpp>
#include <margot/goal.hpp>




// the value of the average
margot::statistical_properties_t avg_full_value;
margot::statistical_properties_t avg_half_value;

class GoalChecking : public CxxTest::TestSuite
{

		margot::monitor_t<int> monitor_full;
		margot::monitor_t<int> monitor_half;

	public:




		/***********************************************
		 * Utility methods
		 ***********************************************/

		static inline bool retriever( margot::statistical_properties_t& value )
		{
			value = avg_full_value;
			return false;
		}


		void setUp( void )
		{
			monitor_full = margot::monitor_t<int>( 3, 3 );
			monitor_full.push(0);
			monitor_full.push(1);
			monitor_full.push(2);
			avg_full_value = monitor_full.average();

			monitor_half = margot::monitor_t<int>( 3, 3 );
			monitor_half.push(0);
			avg_half_value = monitor_half.average();

		}

		void tearDown( void )
		{
			monitor_full.clear();
			monitor_half.clear();
		}


		void test_creation( void )
		{
			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::Less, 1);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::Less, 1);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::Less, 1);

			margot::statistical_properties_t observed_value;
			bool valid = full_goal.observed_value(observed_value);
			TS_ASSERT_DELTA(observed_value, avg_full_value, 1e-7);
			TS_ASSERT(valid);


			valid = half_goal.observed_value(observed_value);
			TS_ASSERT_DELTA(observed_value, avg_half_value, 1e-7);
			TS_ASSERT(!valid);


			valid = static_goal.observed_value(observed_value);
			TS_ASSERT_DELTA(observed_value, avg_full_value, 1e-7);
			TS_ASSERT(!valid);
		}






		/***********************************************
		 * TESTING Greater than goals
		 ***********************************************/

		void test_greater_than_zero( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value - 1;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::Greater, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::Greater, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::Greater, goal_value);


			// test the check
			TS_ASSERT(full_goal.check());
			TS_ASSERT(!half_goal.check());
			TS_ASSERT(static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), std::abs(avg_half_value - goal_value), 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), 0, 1e-7);

		}

		void test_greater_than_below( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value - 0.5f;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::Greater, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::Greater, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::Greater, goal_value);


			// test the check
			TS_ASSERT(full_goal.check());
			TS_ASSERT(!half_goal.check());
			TS_ASSERT(static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), std::abs(avg_half_value - goal_value) / goal_value, 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), 0, 1e-7);

		}

		void test_greater_than_equal( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::Greater, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::Greater, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::Greater, goal_value);


			// test the check
			TS_ASSERT(!full_goal.check());
			TS_ASSERT(!half_goal.check());
			TS_ASSERT(!static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), std::abs(avg_half_value - goal_value) / goal_value, 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), 0, 1e-7);
		}

		void test_greater_than_greater( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value + 1;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::Greater, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::Greater, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::Greater, goal_value);


			// test the check
			TS_ASSERT(!full_goal.check());
			TS_ASSERT(!half_goal.check());
			TS_ASSERT(!static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), std::abs(avg_full_value - goal_value) / goal_value, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), std::abs(avg_half_value - goal_value) / goal_value, 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), std::abs(avg_full_value - goal_value) / goal_value, 1e-7);
		}









		/***********************************************
		 * TESTING Greater or equal than goals
		 ***********************************************/

		void test_greater_or_equal_than_zero( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value - 1;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::Greater, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::Greater, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::Greater, goal_value);


			// test the check
			TS_ASSERT(full_goal.check());
			TS_ASSERT(!half_goal.check());
			TS_ASSERT(static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), std::abs(avg_half_value - goal_value), 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), 0, 1e-7);

		}

		void test_greater_or_equal_than_below( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value - 0.5f;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::GreaterOrEqual, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::GreaterOrEqual, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::GreaterOrEqual, goal_value);


			// test the check
			TS_ASSERT(full_goal.check());
			TS_ASSERT(!half_goal.check());
			TS_ASSERT(static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), std::abs(avg_half_value - goal_value) / goal_value, 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), 0, 1e-7);

		}

		void test_greater_or_equal_than_equal( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::GreaterOrEqual, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::GreaterOrEqual, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::GreaterOrEqual, goal_value);


			// test the check
			TS_ASSERT(full_goal.check());
			TS_ASSERT(!half_goal.check());
			TS_ASSERT(static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), std::abs(avg_half_value - goal_value) / goal_value, 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), 0, 1e-7);
		}

		void test_greater_or_equal_than_greater( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value + 1;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::GreaterOrEqual, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::GreaterOrEqual, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::GreaterOrEqual, goal_value);


			// test the check
			TS_ASSERT(!full_goal.check());
			TS_ASSERT(!half_goal.check());
			TS_ASSERT(!static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), std::abs(avg_full_value - goal_value) / goal_value, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), std::abs(avg_half_value - goal_value) / goal_value, 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), std::abs(avg_full_value - goal_value) / goal_value, 1e-7);
		}


















		/***********************************************
		 * TESTING Less than goals
		 ***********************************************/

		void test_less_than_zero( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value - 1;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::Less, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::Less, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::Less, goal_value);


			// test the check
			TS_ASSERT(!full_goal.check());
			TS_ASSERT(!half_goal.check());
			TS_ASSERT(!static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), std::abs(avg_full_value - goal_value), 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), std::abs(avg_half_value - goal_value), 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), std::abs(avg_full_value - goal_value), 1e-7);

		}

		void test_less_than_below( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value - 0.5f;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::Less, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::Less, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::Less, goal_value);

			// test the check
			TS_ASSERT(!full_goal.check());
			TS_ASSERT(half_goal.check());
			TS_ASSERT(!static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), std::abs(avg_full_value - goal_value) / goal_value, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), std::abs(avg_full_value - goal_value) / goal_value, 1e-7);

		}

		void test_less_than_equal( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::Less, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::Less, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::Less, goal_value);


			// test the check
			TS_ASSERT(!full_goal.check());
			TS_ASSERT(half_goal.check());
			TS_ASSERT(!static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), 0, 1e-7);
		}

		void test_less_than_greater( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value + 1;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::Less, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::Less, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::Less, goal_value);


			// test the check
			TS_ASSERT(full_goal.check());
			TS_ASSERT(half_goal.check());
			TS_ASSERT(static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), 0, 1e-7);
		}












		/***********************************************
		 * TESTING Less or equal than goals
		 ***********************************************/

		void test_less_or_equal_than_zero( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value - 1;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::LessOrEqual, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::LessOrEqual, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::LessOrEqual, goal_value);


			// test the check
			TS_ASSERT(!full_goal.check());
			TS_ASSERT(half_goal.check());
			TS_ASSERT(!static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), std::abs(avg_full_value - goal_value), 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), std::abs(avg_half_value - goal_value), 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), std::abs(avg_full_value - goal_value), 1e-7);

		}

		void test_less_or_equal_than_below( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value - 0.5f;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::LessOrEqual, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::LessOrEqual, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::LessOrEqual, goal_value);

			// test the check
			TS_ASSERT(!full_goal.check());
			TS_ASSERT(half_goal.check());
			TS_ASSERT(!static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), std::abs(avg_full_value - goal_value) / goal_value, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), std::abs(avg_full_value - goal_value) / goal_value, 1e-7);

		}

		void test_less_or_equal_than_equal( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::LessOrEqual, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::LessOrEqual, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::LessOrEqual, goal_value);


			// test the check
			TS_ASSERT(full_goal.check());
			TS_ASSERT(half_goal.check());
			TS_ASSERT(static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), 0, 1e-7);
		}

		void test_less_or_equal_than_greater( void )
		{
			// set the goal value
			margot::statistical_properties_t goal_value = avg_full_value + 1;

			margot::goal_t full_goal(monitor_full, margot::DataFunction::Average, margot::ComparisonFunction::LessOrEqual, goal_value);
			margot::goal_t half_goal(monitor_half, margot::DataFunction::Average, margot::ComparisonFunction::LessOrEqual, goal_value);
			margot::goal_t static_goal(&GoalChecking::retriever, margot::ComparisonFunction::LessOrEqual, goal_value);


			// test the check
			TS_ASSERT(full_goal.check());
			TS_ASSERT(half_goal.check());
			TS_ASSERT(static_goal.check());


			// test the relative error
			TS_ASSERT_DELTA(full_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(half_goal.relative_error(), 0, 1e-7);
			TS_ASSERT_DELTA(static_goal.relative_error(), 0, 1e-7);
		}


};
