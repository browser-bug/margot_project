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



#include <margot/config.hpp>
#include <margot/memory_monitor.hpp>
#include <margot/process_cpu_usage_monitor.hpp>
#include <margot/system_cpu_usage_monitor.hpp>
#include <margot/throughput_monitor.hpp>
#include <margot/time_monitor.hpp>
#include <margot/frequency_monitor.hpp>
#include <margot/energy_monitor.hpp>




class MonitorMeasures : public CxxTest::TestSuite
{

	public:




		void test_time_monitor( void )
		{
			margot::time_monitor_t monitor(margot::TimeMeasure::Microseconds, 1);

			// sleep 5 ms
			monitor.start();
			usleep(5000);
			monitor.stop();

			// check the time measure
			TS_ASSERT_DELTA(monitor.average(), 5000, 500);
		}


		void test_memory_usage( void )
		{
			margot::memory_monitor_t monitor(1);

			// measure the used memory
			monitor.extractMemoryUsage();


			// try to perform some checks
			TS_ASSERT( monitor.average() > 0 );
			TS_ASSERT( monitor.average() < monitor.extractVmPeakSize() );
		}




		void test_proc_cpu_usage( void )
		{
			margot::process_cpu_usage_monitor_t monitor(1);

			// perform some useless stuff
			monitor.start();

			for (size_t i = 0; i < 70000000; ++i)
			{
				volatile double num = 47238.3244 + static_cast<double>(i);
				num = num * num * num;
				num = static_cast<double>(static_cast<size_t>(num) % (i + 1));
			}

			monitor.stop();

			// this might fail, if there is contention on the CPU
			TS_ASSERT_DELTA(monitor.average(), 1.0, 0.3)
		}



		void test_sys_cpu_usage( void )
		{
			margot::system_cpu_usage_monitor_t monitor(1);

			// perform some useless stuff
			monitor.start();

			for (size_t i = 0; i < 70000000; ++i)
			{
				volatile double num = 47238.3244 + static_cast<double>(i);
				num = num * num * num;
				num = static_cast<double>(static_cast<size_t>(num) % (i + 1));
			}

			monitor.stop();

			// it is not really possible to test
			TS_ASSERT(monitor.average() > 0)
			TS_ASSERT(monitor.average() < sysconf( _SC_NPROCESSORS_ONLN ))
		}




		void test_throughput_monitor( void )
		{
			margot::throughput_monitor_t monitor(1);

			// sleep 5 ms
			monitor.start();
			usleep(5000);
			monitor.stop(5);

			// check the time measure
			TS_ASSERT_DELTA(monitor.average(), 1000, 100);
		}



		void test_frequency_monitor( void )
		{
			margot::frequency_monitor_t monitor(1);

			// read the frequency
			monitor.measure();

			// check the time measure
			TS_ASSERT(monitor.average() > 100);
		}

		void test_energy_monitor( void )
		{
			margot::energy_monitor_t monitor(margot::energy_monitor_t::Domain::Cores);

			// start the mesure
			try
			{
				monitor.start();

				usleep(1000);

				monitor.stop();

				const auto measure = monitor.last();

				TS_ASSERT_LESS_THAN(0, measure);
			}
			catch (std::runtime_error& error)
			{
				std::cout << std::endl << "WARNING: RAPL driver not available ";
			}
		}

		void test_energy_monitor2( void )
		{
			margot::energy_monitor_t monitor(margot::energy_monitor_t::Domain::Package);

			// start the mesure
			try
			{
				monitor.start();

				usleep(1000);

				monitor.stop();

				const auto measure = monitor.last();

				TS_ASSERT_LESS_THAN(0, measure);
			}
			catch (std::runtime_error& error)
			{
				std::cout << std::endl << "WARNING: RAPL driver not available ";
			}
		}
};
