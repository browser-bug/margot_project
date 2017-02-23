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
#include <margot/operating_point.hpp>
#include <margot/view.hpp>




class ViewTest : public CxxTest::TestSuite
{

		margot::configuration_map_t points_two;
		margot::configuration_map_t points_five;


	public:


		void setUp( void )
		{
			// initialize the two points list
			points_two =
			{
				{
					{ 1, 2, 3},
					{ 5.0f, 6.0f, 4.0f}
				},
				{
					{ 2, 1, 3},
					{5.0f, 7.0f, 3.0f}
				}
			};


			// initialize the five points list
			points_five =
			{
				{
					{1},
					{1.0f, 1.0f, 5.0f}
				},
				{
					{2},
					{1.0f, 2.0f, 4.0f}
				},
				{
					{3},
					{1.0f, 3.0f, 3.0f}
				},
				{
					{4},
					{1.0f, 4.0f, 2.0f}
				},
				{
					{5},
					{1.0f, 5.0f, 1.0f}
				},
			};

		}




		void test_view_creation_empty( void )
		{
			margot::configuration_map_t ops;
			margot::view_t view = margot::view_t::parameter_view(1, ops);
		}




		void test_view_creation( void )
		{
			// create a view
			margot::view_t view = margot::view_t::metric_view(1, points_two);

			// retrieve the iterators
			margot::view_t::view_range_t stored_ops = view.range();

			// check the first point
			TS_ASSERT_EQUALS(stored_ops.first->second[0], 1);
			TS_ASSERT_EQUALS(stored_ops.first->second[1], 2);
			TS_ASSERT_EQUALS(stored_ops.first->second[2], 3);

			// check the second point
			--stored_ops.second;
			TS_ASSERT_EQUALS(stored_ops.second->second[0], 2);
			TS_ASSERT_EQUALS(stored_ops.second->second[1], 1);
			TS_ASSERT_EQUALS(stored_ops.second->second[2], 3);
		}

		void test_creation_same( void )
		{
			margot::view_t view = margot::view_t::metric_view(0, points_two);

			// retrieve the iterators
			margot::view_t::view_range_t stored_ops = view.range();

			// check the first point
			TS_ASSERT_EQUALS(stored_ops.first->second[0], 1);
			TS_ASSERT_EQUALS(stored_ops.first->second[1], 2);
			TS_ASSERT_EQUALS(stored_ops.first->second[2], 3);

			// check the second point
			--stored_ops.second;
			TS_ASSERT_EQUALS(stored_ops.second->second[0], 2);
			TS_ASSERT_EQUALS(stored_ops.second->second[1], 1);
			TS_ASSERT_EQUALS(stored_ops.second->second[2], 3);
		}


		void test_creation_add( void )
		{
			// create a view
			margot::view_t view = margot::view_t::metric_view(1, points_two);

			// create additional Operating Points
			const margot::operating_points_t ops =
			{
				{
					{2, 3, 4},
					{1.0f, 1.0f, 1.0f}
				},
				{
					{3, 4, 5},
					{1.0f, 2.0f, 1.0f}
				},
				{
					{4, 5, 6},
					{1.0f, 3.0f, 1.0f}
				},
				{
					{5, 6, 7},
					{1.0f, 4.0f, 1.0f}
				}
			};

			// add them to the view
			view.add(ops);

			// retrieve the iterators
			margot::view_t::view_range_t stored_ops = view.range();

			// get the iterator to the begin
			auto it = stored_ops.first;

			// check the first point
			TS_ASSERT_EQUALS(it->second[0], 2);
			TS_ASSERT_EQUALS(it->second[1], 3);
			TS_ASSERT_EQUALS(it->second[2], 4);


			// check the second point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 3);
			TS_ASSERT_EQUALS(it->second[1], 4);
			TS_ASSERT_EQUALS(it->second[2], 5);


			// check the third point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 4);
			TS_ASSERT_EQUALS(it->second[1], 5);
			TS_ASSERT_EQUALS(it->second[2], 6);


			// check the fourth point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 5);
			TS_ASSERT_EQUALS(it->second[1], 6);
			TS_ASSERT_EQUALS(it->second[2], 7);


			// check the fifth point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 1);
			TS_ASSERT_EQUALS(it->second[1], 2);
			TS_ASSERT_EQUALS(it->second[2], 3);

			// check the sixth point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 2);
			TS_ASSERT_EQUALS(it->second[1], 1);
			TS_ASSERT_EQUALS(it->second[2], 3);

		}



		void test_range_middle( void )
		{
			margot::view_t view_up = margot::view_t::parameter_view(0, points_five);
			margot::view_t view_down = margot::view_t::metric_view(2, points_five);

			// get the ranges up
			margot::view_t::view_range_t range_up = view_up.range(2, 4);

			// get the iterator to first element found
			auto it = range_up.first;

			// check the first point
			TS_ASSERT_EQUALS(it->second[0], 2);

			// check the second point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 3);

			// check the third point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 4);

			// check that it is the last
			++it;
			TS_ASSERT(it == range_up.second);

			// get the ranges down
			margot::view_t::view_range_t range_down = view_down.range(2, 4);

			// get the iterator to first element found
			it = range_down.first;

			// check the first point
			TS_ASSERT_EQUALS(it->second[0], 4);

			// check the second point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 3);

			// check the third point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 2);

			// check that it is the last
			++it;
			TS_ASSERT(it == range_down.second);
		}





		void test_range_up( void )
		{
			margot::view_t view_up = margot::view_t::parameter_view(0, points_five);
			margot::view_t view_down = margot::view_t::metric_view(2, points_five);

			// get the ranges up
			margot::view_t::view_range_t range_up = view_up.range(3, 10);

			// get the iterator to first element found
			auto it = range_up.first;

			// check the first point
			TS_ASSERT_EQUALS(it->second[0], 3);

			// check the second point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 4);

			// check the third point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 5);

			// check that it is the last
			++it;
			TS_ASSERT(it == range_up.second);

			// get the whole range
			range_up = view_up.range();
			TS_ASSERT( it == range_up.second);


			// get the ranges up
			margot::view_t::view_range_t range_down = view_down.range(3, 10);

			// get the iterator to first element found
			it = range_down.first;

			// check the first point
			TS_ASSERT_EQUALS(it->second[0], 3);

			// check the second point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 2);

			// check the third point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 1);

			// check that it is the last
			++it;
			TS_ASSERT(it == range_down.second);

			// get the whole range
			range_down = view_down.range();
			TS_ASSERT( it == range_down.second);
		}


		void test_range_down( void )
		{
			margot::view_t view_up = margot::view_t::parameter_view(0, points_five);
			margot::view_t view_down = margot::view_t::metric_view(2, points_five);

			// get the ranges up
			margot::view_t::view_range_t range_up = view_up.range(-3, 3);

			// get the iterator to first element found
			auto it = range_up.first;

			// check the first point
			TS_ASSERT_EQUALS(it->second[0], 1);

			// check the second point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 2);

			// check the third point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 3);

			// check that it is the last
			++it;
			TS_ASSERT(it == range_up.second);


			// get the ranges up
			margot::view_t::view_range_t range_down = view_down.range(-3, 3);

			// get the iterator to first element found
			it = range_down.first;

			// check the first point
			TS_ASSERT_EQUALS(it->second[0], 5);

			// check the second point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 4);

			// check the third point
			++it;
			TS_ASSERT_EQUALS(it->second[0], 3);

			// check that it is the last
			++it;
			TS_ASSERT(it == range_down.second);
		}



		void test_range_out( void )
		{
			margot::view_t view_up = margot::view_t::parameter_view(0, points_five);
			margot::view_t view_down = margot::view_t::metric_view(2, points_five);

			// get the extremes
			margot::view_t::view_range_t extremes_up = view_up.range();
			margot::view_t::view_range_t extremes_down = view_down.range();

			// get the ranges out up
			margot::view_t::view_range_t range_up = view_up.range(100, 150);
			margot::view_t::view_range_t range_down = view_down.range(100, 150);

			// check the result
			TS_ASSERT(range_up.first == extremes_up.second);
			TS_ASSERT(range_up.second == extremes_up.second);
			TS_ASSERT(range_down.first == extremes_down.second);
			TS_ASSERT(range_down.second == extremes_down.second);
			TS_ASSERT_EQUALS(std::distance(range_up.first, range_up.second), 0);
			TS_ASSERT_EQUALS(std::distance(range_down.first, range_down.second), 0);


			// get the ranges out up
			range_up = view_up.range(-100, -150);
			range_down = view_down.range(-100, -150);

			// check the result
			TS_ASSERT(range_up.first == extremes_up.first);
			TS_ASSERT(range_up.second == extremes_up.first);
			TS_ASSERT(range_down.first == extremes_down.first);
			TS_ASSERT(range_down.second == extremes_down.first);
			TS_ASSERT_EQUALS(std::distance(range_up.first, range_up.second), 0);
			TS_ASSERT_EQUALS(std::distance(range_down.first, range_down.second), 0);
		}



		void test_range_all( void )
		{
			margot::view_t view_up = margot::view_t::parameter_view(0, points_five);
			margot::view_t view_down = margot::view_t::metric_view(2, points_five);

			// get the extremes
			margot::view_t::view_range_t extremes_up = view_up.range();
			margot::view_t::view_range_t extremes_down = view_down.range();

			// get the ranges out up
			margot::view_t::view_range_t range_up = view_up.range(-100, 150);
			margot::view_t::view_range_t range_down = view_down.range(-100, 150);


			// check the result
			TS_ASSERT(range_up.first == extremes_up.first);
			TS_ASSERT(range_up.second == extremes_up.second);
			TS_ASSERT(range_down.first == extremes_down.first);
			TS_ASSERT(range_down.second == extremes_down.second);
		}

		void test_extractor( void )
		{
			margot::view_t view = margot::view_t::parameter_view(0, points_five);

			margot::operating_point_t op =
			{
				{6, 7, 8},
				{8.0f, 10.0f, 11.0f}
			};

			TS_ASSERT_EQUALS(view.extract_op_value(op), 6);
		}

		void test_range_value_parameter( void )
		{
			margot::view_t view = margot::view_t::parameter_view(0, points_five);

			margot::margot_value_t min_value = view.get_minimum_value();
			margot::margot_value_t max_value = view.get_maximum_value();

			TS_ASSERT_DELTA(min_value, 1.0f, 0.01f);
			TS_ASSERT_DELTA(max_value, 5.0f, 0.01f);
		}

		void test_range_value_metric( void )
		{
			margot::view_t view = margot::view_t::metric_view(2, points_five);

			margot::margot_value_t min_value = view.get_minimum_value();
			margot::margot_value_t max_value = view.get_maximum_value();

			TS_ASSERT_DELTA(min_value, 1.0f, 0.01f);
			TS_ASSERT_DELTA(max_value, 5.0f, 0.01f);
		}

};
