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


#include<sys/time.h>     // portability
#include<sys/resource.h> // rusage
#include<ctime> // timestamp
#include<stdexcept>
#include <cmath>

#include <margot/process_cpu_usage_monitor.hpp>


using std::chrono::duration_cast;
using std::chrono::duration;


namespace margot
{


	// t3 = t1 - t2
	void tsDifference(timespec t1, timespec t2, timespec& t3)
	{

		unsigned int c = 0;

		if ( t2.tv_nsec < t1.tv_nsec )
		{
			t3.tv_nsec = t1.tv_nsec - t2.tv_nsec;
		}
		else
		{
			t3.tv_nsec = 1000000000 + t1.tv_nsec - t2.tv_nsec;
			c = 1;
		}

		t3.tv_sec = t1.tv_sec - t2.tv_sec - c;
	}


	// utility function, convert a rusage information in the used time
	void rusage_to_timespec(rusage r, timespec& t)
	{
		t.tv_sec = r.ru_stime.tv_sec + r.ru_utime.tv_sec;
		t.tv_nsec = r.ru_stime.tv_usec * 1000 + r.ru_utime.tv_usec * 1000;
	}


#ifdef WITH_HARDWARE_COUNTER
	void getProcessTimeHard(timespec& process_time)
	{

		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &process_time);

	}
#endif


	void getProcessTimeSoft(timespec& process_time)
	{

		rusage process_time_rusage;

		getrusage(RUSAGE_SELF, &process_time_rusage);

		rusage_to_timespec(process_time_rusage, process_time);
	}




	process_cpu_usage_monitor_t::process_cpu_usage_monitor_t(CounterType __attribute__((unused)) counter_type, const std::size_t window_size, const std::size_t min_size): monitor_t( window_size, min_size)
	{

#ifdef WITH_HARDWARE_COUNTER

		switch (counter_type)
		{
			case CounterType::HardwareCounter:
				getProcessTime = getProcessTimeHard;
				break;

			case CounterType::SoftwareCounter:
				getProcessTime = getProcessTimeSoft;
				break;

			default:
				throw std::runtime_error("DEFENSIVE PROGRAMMING: Can't figure out which counter it is used");
				break;
		}

#else
		getProcessTime = getProcessTimeSoft;
#endif

		started = false;
	}



	process_cpu_usage_monitor_t::process_cpu_usage_monitor_t(const size_t window_size, const std::size_t min_size ): monitor_t( window_size, min_size )
	{
		getProcessTime = getProcessTimeSoft;
		started = false;
	}






	void process_cpu_usage_monitor_t::start()
	{

		if (started)
		{
			return;
		}

		started = true;

		tStart = std::chrono::steady_clock::now();
		getProcessTime(uStart);
	}



	void process_cpu_usage_monitor_t::stop()
	{

		// take the stop times
		timespec uStop;
		getProcessTime(uStop);
		std::chrono::steady_clock::time_point tStop = std::chrono::steady_clock::now();

		// check if the monitor has been started
		if (!started)
		{
			return;
		}

		// get the elapsed time in nanoseconds
		uint64_t elapsedTimeSystem = duration_cast<std::chrono::nanoseconds>(tStop - tStart).count();

		// compute the process time difference
		timespec process_difference;
		tsDifference(uStop, uStart, process_difference);

		// convert the difference in nanoseconds
		uint64_t elapsedTimeProcess = static_cast<uint64_t>((process_difference.tv_sec * static_cast<uint64_t>
		                              (1000000000)) + process_difference.tv_nsec);


		// compute the percentage
		value_type percentage = static_cast<value_type>(elapsedTimeProcess) / static_cast<value_type>(elapsedTimeSystem);

		// push the percentage in the data buffer
		push(percentage);

		// change the flag
		started = false;

	}




}
