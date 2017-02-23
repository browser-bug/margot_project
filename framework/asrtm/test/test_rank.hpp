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
#include <margot/rank_calculator.hpp>



class RankTest : public CxxTest::TestSuite
{

		margot::operating_point_t op;


	public:

		void setUp( void )
		{
			op =
			{
				{1, 2, 3},
				{4, 5, 6}
			};
		}


		void test_rank_creation_empty( void )
		{
			margot::rank_calculator_t my_rank;

			TS_ASSERT_EQUALS(my_rank(op), 0);
		}



		void test_rank_creation_linear_one( void )
		{
			margot::rank_calculator_t my_rank;

			my_rank.define_linear_rank(margot::RankObjective::Minimize, margot::rank_metric_t{0, 1.0f});

			TS_ASSERT_EQUALS(my_rank(op), 4);
		}


		void test_rank_creation_linear_two( void )
		{
			margot::rank_calculator_t my_rank;

			my_rank.define_linear_rank(margot::RankObjective::Minimize, margot::rank_metric_t{0, 1.0f}, margot::rank_parameter_t{0, 2.0f});

			TS_ASSERT_EQUALS(my_rank(op), 6);
		}





		void test_rank_creation_geometric_one( void )
		{
			margot::rank_calculator_t my_rank;

			my_rank.define_geometric_rank(margot::RankObjective::Minimize, margot::rank_metric_t{0, 1.0f});

			TS_ASSERT_EQUALS(my_rank(op), 4);
		}


		void test_rank_creation_geometric_two( void )
		{
			margot::rank_calculator_t my_rank;

			my_rank.define_geometric_rank(margot::RankObjective::Minimize, margot::rank_metric_t{0, 2.0f}, margot::rank_parameter_t{0, 1.0f});

			TS_ASSERT_EQUALS(my_rank(op), 16);
		}


};
