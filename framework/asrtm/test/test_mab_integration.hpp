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




class MabIntegrationTest : public CxxTest::TestSuite
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


		void test_mab_simulation( void )
		{
			// define the time orizon
			constexpr std::size_t T = 100;

			// create the monitor that we use to fake the measures
			margot::monitor_t<int> my_arm_monitor;


			// create the goal related to the monitor
			margot::goal_t my_goal(my_arm_monitor, margot::DataFunction::Average, margot::ComparisonFunction::Less, 8);

			// create the Operating Point list
			margot::operating_points_t points =
			{
				{{1}, {8}},
				{{2}, {4}}, // This point is the one selected by the state (normally)
				{{3}, {2}}
			};

			// create the asrtm
			margot::asrtm_t manager;

			// add my awesome learning paramter
			manager.define_learning_sw_ucb_parameters({{0.0f, {7.0f, 8.0f, 9.0f}}}, T / 10);

			// add the Operating Points
			manager.add_operating_points(points);

			// define the rank function
			manager.define_linear_rank(margot::RankObjective::Minimize, margot::rank_parameter_t{0, 1.0f});

			// define the constraint
			manager.add_metric_constraint(my_goal, 0, 10);

			// define a counter for each arm
			std::size_t counter_arm1 = 0;    // BAD ARM     (half value)
			std::size_t counter_arm2 = 0;    // MEDIUM ARM  (same value)
			std::size_t counter_arm3 = 0;    // GOOD ARM    (double value)

			// run the simulation
			for ( std::size_t t = 0; t < T; ++t )
			{
				// get the configuration
				manager.update();
				manager.find_best_operating_point();
				bool conf_changed;
				margot::configuration_t actual_configuration = manager.get_best_configuration(&conf_changed);

				if (conf_changed)
				{
					manager.configuration_applied();
				}


				// get the value of the selected arm
				const int selected_arm = static_cast<int>(actual_configuration.at(1));

				// START MONITOR

				// ELABORATION

				// stop the monitor
				switch (selected_arm)
				{
					case 7:
						my_arm_monitor.push(manager.get_metric_value(0) * 2);
						++counter_arm1;
						break;

					case 8:
						my_arm_monitor.push(manager.get_metric_value(0));
						++counter_arm2;
						break;

					case 9:
						my_arm_monitor.push(manager.get_metric_value(0) / 2);
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
