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
#include <random>
#include <cstddef>

#include <cxxtest/TestSuite.h>


#include <margot/config.hpp>
#include <margot/learning_state.hpp>
#include <margot/mab_sw_ucb.hpp>



class SwMabTest : public CxxTest::TestSuite
{
	public:

		void test_mab_creation( void )
		{
			margot::learning_state_ptr_t mab = margot::learning_state_ptr_t(new margot::sw_mab_t(1000, 0.5f));

			mab->define_knobs({{0.0f, {1, 2, 3}}, {0.0f, {4, 5, 6}}});
		}


		void test_mab_simulation( void )
		{

			// define the time orizon
			constexpr std::size_t T = 100;


			// create the mab
			margot::learning_state_ptr_t mab = margot::learning_state_ptr_t(new margot::sw_mab_t(T / 10, 0.5f));
			mab->define_knobs({{0.0f, {1, 2, 3}}});

			// define the reward distribution for the three knobs
			std::default_random_engine generator;
			std::uniform_int_distribution<int> distribution_arm_1(200, 230);
			std::uniform_int_distribution<int> distribution_arm_2(700, 800);
			std::uniform_int_distribution<int> distribution_arm_3(900, 1000);


			// define a counter for each arm
			std::size_t counter_arm1 = 0;
			std::size_t counter_arm2 = 0;
			std::size_t counter_arm3 = 0;


			// run the simulation
			for ( std::size_t t = 0; t < T; ++t )
			{
				// get the configuration
				auto conf = mab->get_best_configuration();

				// generate a reward
				switch (static_cast<int>(conf.at(0)))
				{
					case 1:
						mab->push_reward(conf, distribution_arm_1(generator));
						++counter_arm1;
						break;

					case 2:
						mab->push_reward(conf, distribution_arm_2(generator));
						++counter_arm2;
						break;

					case 3:
						mab->push_reward(conf, distribution_arm_3(generator));
						++counter_arm3;
						break;

					default:

						// make sure to fail if we got some strange value
						TS_ASSERT(false);
				}
			}

			// std::cout << std::endl;
			// std::cout << "COUNTER ARM 1 (BAD)    -> " << counter_arm1 << std::endl;
			// std::cout << "COUNTER ARM 2 (MEDIUM) -> " << counter_arm2 << std::endl;
			// std::cout << "COUNTER ARM 3 (GOOD)   -> " << counter_arm3 << std::endl;

			TS_ASSERT_LESS_THAN(counter_arm1, counter_arm2);
			TS_ASSERT_LESS_THAN(counter_arm2, counter_arm3);
		}





};
