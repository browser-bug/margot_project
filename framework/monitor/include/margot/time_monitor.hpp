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


#ifndef MARGOT_MONITOR_TIME_MONITOR_HEADER
#define MARGOT_MONITOR_TIME_MONITOR_HEADER

#include <chrono>
#include <cstdint>


#include "margot/config.hpp"
#include "margot/monitor.hpp"



/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{

	/**
	 * @brief The available time measure
	 *
	 * @details
	 * By default the time monitor measures milliseconds
	 */
	enum class TimeMeasure : uint8_t
	{
		Nanoseconds,
		Microseconds,
		Milliseconds,
		Seconds
	};


	/**
	 * @brief  The time monitor
	 *
	 * @details
	 * This monitor uses the std::chrono interface to gather the execution time
	 * using a steady clock, if available. Otherwise it uses the monotonic_clock
	 */
	class time_monitor_t: public monitor_t<unsigned long int>
	{
		public:


			/**
			 * @brief define the time_monitor type
			 */
			using value_type = unsigned long int;


			/****************************************************
			 * Time Monitor methods
			 ****************************************************/

			/**
			 * @brief  Default constructor
			 *
			 * @param [in] window_size The maximum number of elements stored in a monitor
			 *
			 * @details
			 * The default measure is in milliseconds
			 *
			 */
			time_monitor_t(const std::size_t window_size = 1, const std::size_t min_size = 1);

			/**
			 * @brief  Generalized constructor
			 *
			 * @param time_measure The measure unit
			 *
			 * @param window_size The dimension of the observation window
			 *
			 */
			time_monitor_t(TimeMeasure time_measure, const std::size_t window_size = 1, const std::size_t min_size = 1 );

			/**
			 * @brief  Start the observation
			 *
			 */
			void start();


			/**
			 * @brief  Stop the observation and push the new data in the circular buffer
			 *
			 */
			void stop();



		private:

			/**
			 * @brief The starting time when the measure is started
			 */
			std::chrono::steady_clock::time_point t_start;


			/**
			 * @brief States if a measure is started
			 */
			bool started;

			/**
			 * @brief Function used to convert the time interval
			 */
			std::function<value_type(std::chrono::steady_clock::time_point, std::chrono::steady_clock::time_point)>
			time_extractor;

	};



}


#endif // MARGOT_MONITOR_TIME_MONITOR_HEADER
