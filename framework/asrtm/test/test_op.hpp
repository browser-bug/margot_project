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
#include <margot/operating_point.hpp>




class OperatingPointTest : public CxxTest::TestSuite
{

	public:


		void test_op_creation( void )
		{
			// create some operating points
			margot::operating_points_t ops =
			{
				{
					{1, 2, 3},
					{0.3f, 0.2f, 1.0f}
				}
			};


			// create a look-up table
			margot::lookup_table_t table =
			{
				{1, 2, 3},
				{3, 5, 2},
				{22, 55, 43}
			};
		}


		void test_op_creation_empty( void )
		{
			margot::operating_points_t ops;
			margot::lookup_table_t table;
		}
};
