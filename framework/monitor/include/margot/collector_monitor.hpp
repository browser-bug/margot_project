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

#ifndef MARGOT_MONITOR_COLLECTOR_MONITOR_HEADER
#define MARGOT_MONITOR_COLLECTOR_MONITOR_HEADER

#include <string>
#include <memory>

#include "margot/config.hpp"
#include "margot/monitor.hpp"


/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{


	/**
	 * @brief  The collector monitor
	 *
	 * @details
	 * This class is a wrapper of the ETH Zurich monitoring framework.
	 */
	class collector_monitor_t: public monitor_t<double>
	{
		public:

			/**
			 * @brief define the throughput_monitor type
			 */
			using value_type = double;

			/**
			* @brief Wrapper to communicate with the monitor framework
			*/
			class collector_interface
			{
				public:
					virtual void start( void ) = 0;
					virtual void stop( void ) = 0;
					virtual value_type get( void ) = 0;
					virtual ~collector_interface(void) = default;
			};


			/****************************************************
			 * Collector Monitor methods
			 ****************************************************/

			/**
			 * @brief  Default constructor
			 *
			 * @param window_size The dimension of the observation window
			 *
			 * @details
			 * The topic must be a valid Mosquitto topic
			 *
			 */
			collector_monitor_t(const std::size_t window_size = 1, const std::size_t min_size = 1);


			/**
			 * @brief  Default constructor
			 *
			 * @param window_size The dimension of the observation window
			 * @param min_size The minimum number of required element to compute statistical measurement
			 *
			 * @details
			 * The default measure is in Khz FIXME
			 *
			 */
			collector_monitor_t(const std::string topic, const std::string address, const int port, const std::size_t window_size = 1, const std::size_t min_size = 1);


			/**
			 * @brief Destructors
			 *
			 * @details
			 * Clean the sensor structures
			 */
			~collector_monitor_t(void) = default;



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
			 * @brief The data structure to handle the monitor
			 */
			std::shared_ptr<collector_interface> interface;

			/**
			 * @brief States if the measure is started
			 */
			bool started;
	};


}

#endif // MARGOT_MONITOR_COLLECTOR_MONITOR_HEADER
