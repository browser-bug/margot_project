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



#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <unistd.h>
#include <string>



#include "margot/frequency_monitor.hpp"




namespace margot
{


	frequency_monitor_t::frequency_monitor_t(const std::size_t window_size, const std::size_t min_size): monitor_t( window_size, min_size )
	{
		// get the number of the processors
		unsigned int number_cores = 0;


		// find the number of cores
		bool found_core = true;

		while (found_core)
		{
			FILE* ref = fopen(std::string("/sys/devices/system/cpu/cpu" + std::to_string(
			                                  number_cores) + "/cpufreq/scaling_cur_freq").c_str(), "r");

			if (ref == NULL)
			{
				found_core = false;
			}
			else
			{
				interested_core.emplace_back(number_cores);
				++number_cores;
				fclose(ref);
			}
		}


		// check the number of cores
		if (number_cores == 0)
		{
			throw std::runtime_error("[frequency_monitor_t] Error: no cores detected!");
		}
	}


	void frequency_monitor_t::measure()
	{
		// init the measures
		value_type value = 0, avg = 0;


		// take the temperature for every sensor
		for (const auto& cpuid : interested_core)
		{
			// open the file
			FILE* ref = fopen(std::string("/sys/devices/system/cpu/cpu" + std::to_string(
			                                  cpuid) + "/cpufreq/scaling_cur_freq").c_str(), "r");

			// check if the file is open
			if (ref != NULL)
			{
				int result = ::fscanf(ref, "%u\n", &value);

				// check if it's correct
				if (result == EOF)
				{
					fclose(ref);
					throw std::runtime_error("[FREQUENCY_MONITOR]: Unable to read the frequency of core "
					                         + std::to_string(cpuid));
				}

				fclose(ref);
				// summ it
				avg += value / interested_core.size();
			}
		}

		// push the new value
		push(avg);
	}



	void frequency_monitor_t::cores(std::vector< unsigned int > cores)
	{
		// check if we are able to observe the selected cores
		if (pedantic_check())
		{
			// check if the new cores are valid
			bool valid = true;

			// try to parse the file
			for ( const auto& cpuid : cores )
			{
				FILE* ref = fopen(std::string("/sys/devices/system/cpu/cpu" + std::to_string(cpuid) + "/cpufreq/scaling_cur_freq").c_str(), "r");

				if (ref == NULL)
				{
					valid = false;
					break;
				}
				else
				{
					fclose(ref);
				}
			}


			// throw an exception on wrong ids
			if (!valid)
			{
				throw std::runtime_error("[frequency_monitor_t] Error: unable to read from at least  one of the updated cores");
			}
		}

		// update the set of interested cores
		interested_core = std::move(cores);
	}

}
