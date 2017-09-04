/* core/temperature_monitor.hpp
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

#ifndef MARGOT_TEMPERATURE_MONITOR_HDR
#define MARGOT_TEMPERATURE_MONITOR_HDR

#include <sensors/sensors.h>

#include "margot/monitor.hpp"

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
  class TemperatureMonitor: public Monitor<long long int>
  {


    public:


      /**
       * @brief define the type of the elements stored in the monitor
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
      TemperatureMonitor( const std::size_t window_size = 1 );


      /**
       * @brief  Get the temperature of the CPU executing the calling thread
       */
      inline void measure()
      {
        Monitor::push(TemperatureMonitor::TemperatureSensor::get_instance().measure());
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
      class TemperatureSensor
      {


        public:


          /**
           * @brief Get the insance of the sensor
           *
           * @return The reference of a temperature sensor
           */
          static TemperatureSensor& get_instance( void )
          {
            static TemperatureSensor singleton;
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
          TemperatureSensor( const TemperatureSensor& source) = delete;


          /**
           * @brief Move constructor
           *
           * @param [in] source The source sensor
           *
           * @details
           * Since there is at most one instance of the class, this method
           * is deleted.
           */
          TemperatureSensor( TemperatureSensor&& source) = delete;


          /**
           * @brief Assign operator (copy semantics)
           *
           * @param [in] source The source sensor
           *
           * @details
           * Since there is at most one instance of the class, this method
           * is deleted.
           */
          TemperatureSensor& operator=( const TemperatureSensor& source) = delete;


          /**
           * @brief Assign operator (copy semantics)
           *
           * @param [in] source The source sensor
           *
           * @details
           * Since there is at most one instance of the class, this method
           * is deleted.
           */
          TemperatureSensor& operator=( TemperatureSensor&& source) = delete;


          /**
           * @brief The method used to retrieve the temperature
           *
           * @return The average temperature of the system
           */
          TemperatureMonitor::value_type measure( void );


          /**
           * @brief The deconstructor
           */
          ~TemperatureSensor( void );


        private:


          /**
           * @brief Default constructor
           *
           * @details
           * In this way it is not possible to create more copies of the sensor
           */
          TemperatureSensor( void );


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

#endif // MARGOT_TEMPERATURE_MONITOR_HDR
