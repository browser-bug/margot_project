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


#include <margot/system_cpu_usage_monitor.hpp>


#include <cstddef>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <memory>




namespace margot
{


	inline void readTime(uint64_t* user, uint64_t* nice, uint64_t* system, uint64_t* idle)
	{

		std::shared_ptr<FILE> fp(fopen("/proc/stat", "r"), fclose);
		uint64_t io_wait, irq, softirq, steal, guest, guest_nice = 0;
		int result = ::fscanf(fp.get(), "%*s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", user, nice, system,
		                      idle, &io_wait, &irq, &softirq, &steal, &guest, &guest_nice);

		system += irq + softirq + steal + guest + guest_nice + io_wait;

		// check if it's correct
		if (result == EOF)
		{
			throw std::runtime_error("[system_cpu_usage_t] Error: unable to parse the /proc/stat file: reached end of file.");
		}
	}

	inline void checkTotal(uint64_t total)
	{

		if (total == 0)
		{
			throw std::runtime_error("[system_cpu_usage_t] Error: no time elapsed (interval too short?)");
		}
	}


	system_cpu_usage_monitor_t::system_cpu_usage_monitor_t(const std::size_t window_size, const std::size_t min_size): monitor_t( window_size, min_size )
	{
		started = false;
	}



	void system_cpu_usage_monitor_t::start()
	{

		if (started)
		{
			return;
		}

		started = true;

		// save the current measure
		uint64_t user, sys, nice, idle;
		readTime(&user, &nice, &sys, &idle);
		busy_time = user + sys + nice;
		total_time = busy_time + idle;

	}



	void system_cpu_usage_monitor_t::stop()
	{
		// get the new measure
		uint64_t user, sys, nice, idle, total, busy;
		readTime(&user, &nice, &sys, &idle);
		busy = user + sys + nice;
		total = busy + idle;

		if (!started)
		{
			return;

		}

		started = false;

		// subtract the previous one
		total -= total_time;
		busy -= busy_time;
		checkTotal(total);

		// push the cpu usage into the
		value_type percentage = static_cast<value_type>(busy) / static_cast<value_type>(total);

		push(percentage * sysconf( _SC_NPROCESSORS_ONLN ));
	}





}
