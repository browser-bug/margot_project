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


#ifndef MARGOT_MONITOR_MEMORY_MONITOR_HEADER
#define MARGOT_MONITOR_MEMORY_MONITOR_HEADER

#include <cstddef>

#include "margot/config.hpp"
#include "margot/monitor.hpp"


/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{



	/**
	 * @brief  The Memory Monitor
	 *
	 * @template The type of the monitor
	 *
	 * @details
	 * This class represent a monitor that observe the memory used by the application.
	 * Moreover it provide a methods that get the peak virtual memory used
	 */
	class memory_monitor_t: public monitor_t<std::size_t>
	{
		public:

			/**
			 * @brief define the throughput_monitor type
			 */
			using value_type = std::size_t;




			/****************************************************
			 * Memory Monitor methods
			 ****************************************************/

			/**
			 * @brief  The Memory Monitor constructor
			 *
			 * @param window_size The dimension of the observation window
			 *
			 * @details
			 * The default window size is 1
			 *
			 */
			memory_monitor_t(const std::size_t window_size = 1, const std::size_t min_size = 1);

			/**
			 * @brief  read the memory usage of the application
			 *
			 * @details
			 * The measure of the memory is extracted by parsing the /proc/self/statm
			 * metafile. For this reason the measure doesn't require a start()/stop().
			 * The measure is pushed inside the data buffer
			 */
			void extractMemoryUsage();

			/**
			 * @brief  get the virtual memory peak
			 *
			 * @return the virtual memory peak
			 *
			 * @note
			 * This value is not stored inside the data buffer
			 */
			value_type extractVmPeakSize();

	};



}



#endif // MARGOT_MONITOR_MEMORY_MONITOR_HEADER
