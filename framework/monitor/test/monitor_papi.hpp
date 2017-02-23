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
#include <unistd.h>
#include <iostream>

#include <cxxtest/TestSuite.h>


#include <margot/papi_monitor.hpp>




class MonitorMeasuresPapi : public CxxTest::TestSuite
{

	public:

		void test_papi_monitor( void )
		{
			try
			{
				margot::papi_monitor_t monitor(margot::PapiEvent::CYC_TOT);


				// perform some useless stuff
				monitor.start();

				for (size_t i = 0; i < 700000; ++i)
				{
					double num = 47238.3244 + static_cast<double>(i);
					num = num * num * num;
					num = static_cast<double>(static_cast<size_t>(num) % (i + 1));
				}

				monitor.stop();
				TS_ASSERT_LESS_THAN(0, monitor.average());
			}
			catch ( std::exception& e)
			{
				// It's fine, the testing hardware counter is not available
			}
		}

};
