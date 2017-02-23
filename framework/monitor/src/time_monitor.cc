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



#include "margot/time_monitor.hpp"


#include <ratio>
#include <stdexcept>


using std::chrono::duration_cast;
using std::chrono::duration;


namespace margot
{


	time_monitor_t::value_type getElapsedTimeNanoseconds(std::chrono::steady_clock::time_point tStart,
	        std::chrono::steady_clock::time_point tStop)
	{
		time_monitor_t::value_type elapsedTime = duration_cast<std::chrono::nanoseconds>(tStop - tStart).count();
		return elapsedTime;
	}

	time_monitor_t::value_type getElapsedTimeMicroseconds(std::chrono::steady_clock::time_point tStart,
	        std::chrono::steady_clock::time_point tStop)
	{
		time_monitor_t::value_type elapsedTime = duration_cast<std::chrono::microseconds>(tStop - tStart).count();
		return elapsedTime;
	}

	time_monitor_t::value_type getElapsedTimeMilliseconds(std::chrono::steady_clock::time_point tStart,
	        std::chrono::steady_clock::time_point tStop)
	{
		time_monitor_t::value_type elapsedTime = duration_cast<std::chrono::milliseconds>(tStop - tStart).count();
		return elapsedTime;
	}

	time_monitor_t::value_type getElapsedTimeSeconds(std::chrono::steady_clock::time_point tStart,
	        std::chrono::steady_clock::time_point tStop)
	{
		time_monitor_t::value_type elapsedTime = duration_cast<std::chrono::seconds>(tStop - tStart).count();
		return elapsedTime;
	}



	time_monitor_t::time_monitor_t(TimeMeasure time_measure,
	                               const std::size_t window_size,
	                               const std::size_t min_size): monitor_t(window_size, min_size)
	{

		switch (time_measure)
		{
			case TimeMeasure::Microseconds:
				time_extractor = getElapsedTimeMicroseconds;
				break;

			case TimeMeasure::Milliseconds:
				time_extractor = getElapsedTimeMilliseconds;
				break;

			case TimeMeasure::Seconds:
				time_extractor = getElapsedTimeSeconds;
				break;

			case TimeMeasure::Nanoseconds:
				time_extractor = getElapsedTimeNanoseconds;

			default:
				throw std::logic_error("DEFENSIVE PROGRAMMING: Undefined TimeMeasure in the time monitor");
		}

		started = false;
	}


	time_monitor_t::time_monitor_t(const size_t window_size, const std::size_t min_size): monitor_t(window_size, min_size)
	{
		time_extractor = getElapsedTimeMilliseconds;
		started = false;
	}


	void time_monitor_t::start()
	{

		if (started)
		{
			return;
		}

		started = true;

		t_start = std::chrono::steady_clock::now();

	}



	void time_monitor_t::stop()
	{

		if (!started)
		{
			return;
		}

		time_monitor_t::value_type time_elapsed = time_extractor(t_start, std::chrono::steady_clock::now());


		monitor_t::push(time_elapsed);

		started = false;

	}



}
