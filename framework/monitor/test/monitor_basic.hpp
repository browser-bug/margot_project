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

#include <stdexcept>

#include <cxxtest/TestSuite.h>



#include <margot/monitor.hpp>


class MonitorBasicFunctionalities : public CxxTest::TestSuite
{

	public:




		void test_declaration( void )
		{
			margot::monitor_t<int> monitor_int;
			margot::monitor_t<unsigned int> monitor_uint;
			margot::monitor_t<float> monitor_float;
			margot::monitor_t<double> monitor_double;
		}
















		/***********************************************
		 * TESTING the manipulation of the circular buffer
		 ***********************************************/

		void test_pushing_1( void )
		{
			margot::monitor_t<int> monitor;
			TS_ASSERT_EQUALS(monitor.size(), 0);
			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.size(), 1);
			TS_ASSERT_EQUALS(monitor.last(), 0);
			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.size(), 1);
			TS_ASSERT_EQUALS(monitor.last(), 1);
			monitor.push(2);
			TS_ASSERT_EQUALS(monitor.size(), 1);
			TS_ASSERT_EQUALS(monitor.last(), 2);
		}

		void test_pushing_5( void )
		{
			margot::monitor_t<int> monitor(5);
			TS_ASSERT_EQUALS(monitor.size(), 0);
			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.size(), 1);
			TS_ASSERT_EQUALS(monitor.last(), 0);
			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.size(), 2);
			TS_ASSERT_EQUALS(monitor.last(), 1);
			monitor.push(2);
			TS_ASSERT_EQUALS(monitor.size(), 3);
			TS_ASSERT_EQUALS(monitor.last(), 2);
			monitor.push(3);
			TS_ASSERT_EQUALS(monitor.size(), 4);
			TS_ASSERT_EQUALS(monitor.last(), 3);
			monitor.push(4);
			TS_ASSERT_EQUALS(monitor.size(), 5);
			TS_ASSERT_EQUALS(monitor.last(), 4);
			monitor.push(5);
			TS_ASSERT_EQUALS(monitor.size(), 5);
			TS_ASSERT_EQUALS(monitor.last(), 5);
			monitor.push(6);
			TS_ASSERT_EQUALS(monitor.size(), 5);
			TS_ASSERT_EQUALS(monitor.last(), 6);
			monitor.push(7);
			TS_ASSERT_EQUALS(monitor.size(), 5);
			TS_ASSERT_EQUALS(monitor.last(), 7);
		}

		void test_clear( void )
		{
			margot::monitor_t<int> monitor(5);
			monitor.push(0);
			monitor.push(1);
			monitor.push(2);
			monitor.push(3);
			monitor.push(4);

			TS_ASSERT_EQUALS(monitor.size(), 5);
			monitor.clear();
			TS_ASSERT_EQUALS(monitor.size(), 0);
		}











		/***********************************************
		 * TESTING the average
		 ***********************************************/

		void test_average_int1( void )
		{
			margot::monitor_t<int> monitor(1);
			TS_ASSERT_EQUALS(monitor.average(), 0);

			monitor.push(-1);
			TS_ASSERT_EQUALS(monitor.average(), -1);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.average(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.average(), 1);
		}

		void test_average_int3( void )
		{
			margot::monitor_t<int> monitor(3);
			TS_ASSERT_EQUALS(monitor.average(), 0);

			monitor.push(-1);
			TS_ASSERT_EQUALS(monitor.average(), -1);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.average(), -0.5);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.average(), 0);
		}

		void test_average_uint1( void )
		{
			margot::monitor_t<unsigned int> monitor(1);
			TS_ASSERT_EQUALS(monitor.average(), 0);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.average(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.average(), 1);

			monitor.push(2);
			TS_ASSERT_EQUALS(monitor.average(), 2);
		}

		void test_average_uint3( void )
		{
			margot::monitor_t<unsigned int> monitor(3);
			TS_ASSERT_EQUALS(monitor.average(), 0);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.average(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.average(), 0.5);

			monitor.push(2);
			TS_ASSERT_EQUALS(monitor.average(), 1);
		}

		void test_average_float1( void )
		{
			margot::monitor_t<float> monitor(1);
			TS_ASSERT_EQUALS(monitor.average(), 0);

			monitor.push(-0.9f);
			TS_ASSERT_EQUALS(monitor.average(), -0.9f);

			monitor.push(0.0f);
			TS_ASSERT_EQUALS(monitor.average(), 0.0f);

			monitor.push(1.3f);
			TS_ASSERT_EQUALS(monitor.average(), 1.3f);
		}

		void test_average_float3( void )
		{
			margot::monitor_t<float> monitor(3);
			TS_ASSERT_EQUALS(monitor.average(), 0);

			monitor.push(-0.9f);
			TS_ASSERT_EQUALS(monitor.average(), -0.9f);

			monitor.push(0.1f);
			TS_ASSERT_DELTA(monitor.average(), -0.4f, 1e-7);

			monitor.push(1.7f);
			TS_ASSERT_DELTA(monitor.average(), 0.3f, 1e-7);
		}




















		/***********************************************
		 * TESTING the variance
		 ***********************************************/

		void test_variance_int1( void )
		{
			margot::monitor_t<int> monitor(1);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(-1);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.variance(), 0);
		}

		void test_variance_int3( void )
		{
			margot::monitor_t<int> monitor(3);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(-1);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.variance(), 0.5f);

			monitor.push(1);
			TS_ASSERT_DELTA(monitor.variance(), 1.0f, 1e-7);
		}

		void test_variance_uint1( void )
		{
			margot::monitor_t<unsigned int> monitor(1);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(2);
			TS_ASSERT_EQUALS(monitor.variance(), 0);
		}

		void test_variance_uint3( void )
		{
			margot::monitor_t<unsigned int> monitor(3);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.variance(), 0.5);

			monitor.push(2);
			TS_ASSERT_DELTA(monitor.variance(), 1.0f, 1e-7);
		}

		void test_variance_float1( void )
		{
			margot::monitor_t<float> monitor(1);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(-0.9f);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(0.0f);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(1.3f);
			TS_ASSERT_EQUALS(monitor.variance(), 0);
		}

		void test_variance_float3( void )
		{
			margot::monitor_t<float> monitor(3);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(-0.9f);
			TS_ASSERT_EQUALS(monitor.variance(), 0);

			monitor.push(0.1f);
			TS_ASSERT_DELTA(monitor.variance(), 0.5f, 1e-7);

			monitor.push(1.7f);
			TS_ASSERT_DELTA(monitor.variance(), 1.72f, 1e-7);
		}





		/***********************************************
		 * TESTING the maximum
		 ***********************************************/

		void test_maximum_int1( void )
		{
			margot::monitor_t<int> monitor(1);
			TS_ASSERT_EQUALS(monitor.max(), 0);

			monitor.push(-1);
			TS_ASSERT_EQUALS(monitor.max(), -1);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.max(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.max(), 1);
		}

		void test_maximum_int3( void )
		{
			margot::monitor_t<int> monitor(3);
			TS_ASSERT_EQUALS(monitor.max(), 0);

			monitor.push(-1);
			TS_ASSERT_EQUALS(monitor.max(), -1);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.max(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.max(), 1);
		}

		void test_maximum_uint1( void )
		{
			margot::monitor_t<unsigned int> monitor(1);
			TS_ASSERT_EQUALS(monitor.max(), 0);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.max(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.max(), 1);

			monitor.push(2);
			TS_ASSERT_EQUALS(monitor.max(), 2);
		}

		void test_maximum_uint3( void )
		{
			margot::monitor_t<unsigned int> monitor(3);
			TS_ASSERT_EQUALS(monitor.max(), 0);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.max(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.max(), 1);

			monitor.push(2);
			TS_ASSERT_EQUALS(monitor.max(), 2);
		}

		void test_maximum_float1( void )
		{
			margot::monitor_t<float> monitor(1);
			TS_ASSERT_EQUALS(monitor.max(), 0);

			monitor.push(-0.9f);
			TS_ASSERT_EQUALS(monitor.max(), -0.9f);

			monitor.push(0.0f);
			TS_ASSERT_EQUALS(monitor.max(), 0.0f);

			monitor.push(1.3f);
			TS_ASSERT_EQUALS(monitor.max(), 1.3f);
		}

		void test_maximum_float3( void )
		{
			margot::monitor_t<float> monitor(3);
			TS_ASSERT_EQUALS(monitor.max(), 0);

			monitor.push(-0.9f);
			TS_ASSERT_EQUALS(monitor.max(), -0.9f);

			monitor.push(0.0f);
			TS_ASSERT_EQUALS(monitor.max(), 0.0f);

			monitor.push(1.3f);
			TS_ASSERT_EQUALS(monitor.max(), 1.3f);
		}













		/***********************************************
		 * TESTING the minimum
		 ***********************************************/

		void test_minimum_int1( void )
		{
			margot::monitor_t<int> monitor(1);
			TS_ASSERT_EQUALS(monitor.min(), 0);

			monitor.push(-1);
			TS_ASSERT_EQUALS(monitor.min(), -1);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.min(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.min(), 1);
		}

		void test_minimum_int3( void )
		{
			margot::monitor_t<int> monitor(3);
			TS_ASSERT_EQUALS(monitor.min(), 0);

			monitor.push(-1);
			TS_ASSERT_EQUALS(monitor.min(), -1);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.min(), -1);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.min(), -1);
		}

		void test_minimum_uint1( void )
		{
			margot::monitor_t<unsigned int> monitor(1);
			TS_ASSERT_EQUALS(monitor.min(), 0);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.min(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.min(), 1);

			monitor.push(2);
			TS_ASSERT_EQUALS(monitor.min(), 2);
		}

		void test_minimum_uint3( void )
		{
			margot::monitor_t<unsigned int> monitor(3);
			TS_ASSERT_EQUALS(monitor.min(), 0);

			monitor.push(0);
			TS_ASSERT_EQUALS(monitor.min(), 0);

			monitor.push(1);
			TS_ASSERT_EQUALS(monitor.min(), 0);

			monitor.push(2);
			TS_ASSERT_EQUALS(monitor.min(), 0);
		}

		void test_minimum_float1( void )
		{
			margot::monitor_t<float> monitor(1);
			TS_ASSERT_EQUALS(monitor.min(), 0);

			monitor.push(-0.9f);
			TS_ASSERT_EQUALS(monitor.min(), -0.9f);

			monitor.push(0.0f);
			TS_ASSERT_EQUALS(monitor.min(), 0.0f);

			monitor.push(1.3f);
			TS_ASSERT_EQUALS(monitor.min(), 1.3f);
		}

		void test_minimum_float3( void )
		{
			margot::monitor_t<float> monitor(3);
			TS_ASSERT_EQUALS(monitor.min(), 0);

			monitor.push(-0.9f);
			TS_ASSERT_EQUALS(monitor.min(), -0.9f);

			monitor.push(0.0f);
			TS_ASSERT_EQUALS(monitor.min(), -0.9f);

			monitor.push(1.3f);
			TS_ASSERT_EQUALS(monitor.min(), -0.9f);
		}
};
