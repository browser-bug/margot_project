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

#include <margot/throughput_monitor.hpp>


using std::chrono::duration_cast;
using std::chrono::duration;



namespace margot
{



	throughput_monitor_t::throughput_monitor_t(const std::size_t window_size, const std::size_t min_size): monitor_t(window_size, min_size)
	{
		started = false;
	}



	void throughput_monitor_t::start()
	{

		if (started)
		{
			return;
		}

		tStart = std::chrono::steady_clock::now();

		started = true;
	}


	void throughput_monitor_t::stop(float data)
	{

		std::chrono::steady_clock::time_point tStop = std::chrono::steady_clock::now();

		if (!started)
		{
			return;

		}

		uint64_t elapsed_time = duration_cast<std::chrono::microseconds>(tStop - tStart).count();

		if (pedantic_check())
		{
			if (elapsed_time == 0)
			{
				throw std::runtime_error("[throughput_monitor_t] Error: the observed functionality should last at least 1us");
			}
		}

		push(data * (1000000.0f / elapsed_time));

		started = false;
	}





}
