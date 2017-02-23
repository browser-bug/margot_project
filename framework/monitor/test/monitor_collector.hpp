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
#include <chrono>
#include <thread>
#include <iostream>

#include <cxxtest/TestSuite.h>


#include <margot/collector_monitor.hpp>




class MonitorMeasuresCollector : public CxxTest::TestSuite
{

	public:

		void test_collector_monitor( void )
		{
			try
			{
				// declare the monitor
				margot::collector_monitor_t monitor("antarex/testcluster/testnode/#", "127.0.0.1", 1883);

				// sleep for a little while
				std::this_thread::sleep_for(std::chrono::milliseconds(500));

				// start the reading
				monitor.start();

				// sleep for a little while
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));

				// stop the reading
				monitor.stop();

				// check the result
				TS_ASSERT(monitor.average() != 0);
			}
			catch (std::runtime_error e)
			{
				std::cout << std::endl << "WARNING: Collector didn't receive any information from 127.0.0.1:1883 about \"antarex/testcluster/testnode/#\"" << std::endl;
			}
		}

};
