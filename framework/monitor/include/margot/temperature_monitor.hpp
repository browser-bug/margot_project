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



#ifndef MARGOT_MONITOR_TEMPERATURE_MONITOR_HEADER
#define MARGOT_MONITOR_TEMPERATURE_MONITOR_HEADER

#include <sensors/sensors.h>

#include "margot/config.hpp"
#include "margot/monitor.hpp"


/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{

	/**
	 * @brief  The temperature monitor
	 *
	 * @todo Different temperature for each core
	 *
	 * @details
	 * All the measures are expressed in celsius degrees
	 */
	class temperature_monitor_t: public monitor_t<long long int>
	{
		public:


			/**
			 * @brief define the throughput_monitor type
			 */
			using value_type = long long int;






			/****************************************************
			 * Temperature Monitor methods
			 ****************************************************/


			/**
			 * @brief  Default constructor
			 *
			 * @param window_size The dimension of the observation window
			 *
			 * @details
			 * The default measure is in celsius degrees
			 *
			 */
			temperature_monitor_t(const std::size_t window_size = 1, const std::size_t min_size = 1);

			/**
			 * @brief Destructors
			 *
			 * @details
			 * Clean the sensor structures
			 */
			~temperature_monitor_t(void) = default;

			/**
			 * @brief Copy constructor
			 *
			 * @param [in] source The source asrtm
			 */
			temperature_monitor_t( const temperature_monitor_t& source ) = default;

			/**
			 * @brief Move constructor
			 *
			 * @param [in] source The source asrtm
			 */
			temperature_monitor_t( temperature_monitor_t&& source ) = default;

			/**
			 * @brief Assign operator (copy semantics)
			 *
			 * @param [in] source The source asrtm
			 */
			temperature_monitor_t& operator=(const temperature_monitor_t& source ) = default;

			/**
			 * @brief Assign operator (move semantics)
			 *
			 * @param [in] source The source asrtm
			 */
			temperature_monitor_t& operator=( temperature_monitor_t&& source ) = default;




			/**
			 * @brief  Get the temperature of the CPU executing the calling thread
			 */
			inline void measure()
			{
				monitor_t::push(temperature_monitor_t::temperature_sensor_t::get_instance().measure());
			}



		private:




			/**
			 * @brief The actual sensor for the temperature
			 *
			 * @details
			 * since the monitor exploits the libsensor library for retrieving the actual
			 * temperatures from the hardware sensors, this class gather those measure.
			 * In the current implementation this class averages out the measure from all
			 * the hardware sensors.
			 * For this reason there is a single instance of this sensor among all the
			 * temperature monitors.
			 */
			class temperature_sensor_t
			{
				public:

					/**
					 * @brief Get the insance of the sensor
					 *
					 * @return The reference of a temperature sensor
					 */
					static temperature_sensor_t& get_instance( void )
					{
						static temperature_sensor_t singleton;
						return singleton;
					}

					/**
					 * @brief Copy constructor
					 *
					 * @param [in] source The source sensor
					 *
					 * @details
					 * Since there is at most one instance of the class, this method
					 * is deleted.
					 */
					temperature_sensor_t( const temperature_sensor_t& source) = delete;

					/**
					 * @brief Move constructor
					 *
					 * @param [in] source The source sensor
					 *
					 * @details
					 * Since there is at most one instance of the class, this method
					 * is deleted.
					 */
					temperature_sensor_t( temperature_sensor_t&& source) = delete;

					/**
					 * @brief Assign operator (copy semantics)
					 *
					 * @param [in] source The source sensor
					 *
					 * @details
					 * Since there is at most one instance of the class, this method
					 * is deleted.
					 */
					temperature_sensor_t& operator=( const temperature_sensor_t& source) = delete;

					/**
					 * @brief Assign operator (copy semantics)
					 *
					 * @param [in] source The source sensor
					 *
					 * @details
					 * Since there is at most one instance of the class, this method
					 * is deleted.
					 */
					temperature_sensor_t& operator=( temperature_sensor_t&& source) = delete;


					/**
					 * @brief The method used to retrieve the temperature
					 *
					 * @return The average temperature of the system
					 */
					temperature_monitor_t::value_type measure( void );


					/**
					 * @brief The deconstructor
					 */
					~temperature_sensor_t( void );



				private:

					/**
					 * @brief Default constructor
					 *
					 * @details
					 * In this way it is not possible to create more copies of the sensor
					 */
					temperature_sensor_t( void );


					/**
					 * @brief the number of the sensor
					 */
					const int ns;



					/**
					 * @brief the number of the cores
					 */
					const int nc;


					/**
					 * @brief Store informations about the available sensors
					 */
					typedef struct
					{
						int nr;                       /* Sensor id */
						sensors_chip_name const* cn;  /* Chip name from libsensors */
						int temp_input;               /* Subfeature number from libsensors */
						double temp_crit;             /* Reading of the critical temperature */
						unsigned int ncpus;           /* Number of CPU ids associated with this sensor */
					} core_sensor_t;


					/**
					 * @brief Stores all the available sensors
					 */
					std::vector<core_sensor_t> sensors;

			};

	};




}




#endif // MARGOT_MONITOR_TEMPERATURE_MONITOR_HEADER
