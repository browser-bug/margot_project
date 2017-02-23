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

#ifndef MARGOT_MONITOR_ENERGY_MONITOR_HEADER
#define MARGOT_MONITOR_ENERGY_MONITOR_HEADER

#include <functional>

#include "margot/config.hpp"
#include "margot/monitor.hpp"


/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{


	/**
	 * @brief  The energy monitor
	 *
	 * @details
	 * All the measures are expressed in uJ
	 * For the measurement it uses the rapl framework
	 */
	class energy_monitor_t: public monitor_t<long double>
	{
		public:

			enum class Domain : unsigned int
			{
				Cores,
				Uncores,
				Ram,
				Package
			};

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
			energy_monitor_t(const std::size_t window_size = 1, const std::size_t min_size = 1);


			/**
			 * @brief  Default constructor
			 *
			 * @param window_size The dimension of the observation window
			 *
			 * @details
			 * The default measure is in Khz FIXME
			 *
			 */
			energy_monitor_t(const Domain interested_domain, const std::size_t window_size = 1, const std::size_t min_size = 1, const std::vector<int> target_packages = {});


			/**
			 * @brief Destructors
			 *
			 * @details
			 * Clean the sensor structures
			 */
			~energy_monitor_t(void) = default;



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
			 * @brief A function to extract a metric
			 */
			std::function<std::vector <std::pair<unsigned long long int, unsigned long long int>> (void)> extractor;


			/**
			 * @brief The previous measure
			 */
			std::vector <std::pair<unsigned long long int, unsigned long long int>> previous_measure;

			/**
			 * @brief States if the measure is started
			 */
			bool started;
	};


}

#endif // MARGOT_MONITOR_ENERGY_MONITOR_HEADER
