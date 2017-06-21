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

#ifndef MARGOT_MONITOR_ODROID_ENERGY_MONITOR_HEADER
#define MARGOT_MONITOR_ODROID_ENERGY_MONITOR_HEADER

#include <functional>
#include <thread>
#include <iostream>
#include "margot/config.hpp"
#include "margot/monitor.hpp"

/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{
	void synchronous_power_call(const uint64_t polling_time_ms, bool &started, bool &end_monitor, long double &total_energy);

	class synchronous_thread_if
	{
			
		private:
	
		/**
		 * @brief Definition of the synchronous threads
		 */
		std::thread synchronous_thread;
		//std::shared_ptr<std::thread> synchronous_thread_p;
			
		/**
	 	* @brief States if the measure is started
	 	*/
		long double total_energy;

		/**
	 	* @brief States if the monitor should be synchronous thread should be closed
	 	*/
		bool end_monitor;

		/**
	 	* @brief States if the measure is started
	 	*/
		bool started;				

		public:
				
			synchronous_thread_if(const uint64_t polling_time_ms)
			{
				total_energy=0;
				started = false;
				end_monitor = false;
				synchronous_thread = std::thread(synchronous_power_call, polling_time_ms, std::ref(started), std::ref(end_monitor), std::ref(total_energy));
			}

			~synchronous_thread_if(void)					
			{
				end_monitor = true;
				synchronous_thread.join();
				std::cout << " thread joined " << std::endl;
			}

			void start(){
				started = true;
				total_energy = 0;
			}
					
			long double stop(){
				started = false;
				return total_energy;
			}

	};

	/**
	 * @brief  The energy monitor
	 *
	 * @details
	 * All the measures are expressed in mJ
	 * For the measurement it uses a discrete integration over the power measures
	 */
	class odroid_energy_monitor_t: public monitor_t<long double>
	{
		public:

			/**
			 * @brief define the throughput_monitor type
			 */
			using value_type = long double;

			/****************************************************
			 * Energy Monitor methods
			 ****************************************************/

			/**
			 * @brief  Default constructor
			 *
			 * @param window_size The dimension of the observation window
			 *
			 * @details
			 * The default measure is in Khz
			 *
			 */
	//		odroid_energy_monitor_t(const std::size_t window_size = 1, const std::size_t min_size = 1);
			odroid_energy_monitor_t(const std::size_t window_size = 1, const std::size_t min_size = 1);


			/**
			 * @brief  Default constructor
			 *
			 * @param window_size The dimension of the observation window
			 *
			 * @details
			 * The default measure is in Khz FIXME
			 *
			 */
			odroid_energy_monitor_t(TimeMeasure time_measure, const uint64_t polling_time_ms, const std::size_t window_size = 1, const std::size_t min_size = 1);


			/**
			 * @brief Destructors
			 *
			 * @details
			 * Clean the sensor structures
			 */
			~odroid_energy_monitor_t(void) = default;
			//~odroid_energy_monitor_t(void);


			/**
			 * @brief Start the measure of energy
			 */
			void start( void );



			/**
			 * @brief Stop the measure of energy
			 */
			void stop( void );


		private:

			/**
			 * @brief Definition of the synchronous threads
			 */
			std::shared_ptr<synchronous_thread_if> synchronous_thread_if_p;
			/*
			* @brief States if the measure is started
			*/
			bool started;			

	};


}

#endif // MARGOT_MONITOR_ODROID_ENERGY_MONITOR_HEADER
