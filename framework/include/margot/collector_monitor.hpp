/* core/collector_monitor.hpp
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

#ifndef MARGOT_COLLECTOR_MONITOR_HDR
#define MARGOT_COLLECTOR_MONITOR_HDR

#include <string>
#include <memory>

#include "margot/monitor.hpp"

namespace margot
{

  /**
   * @brief  The collector monitor
   *
   * @details
   * This class is a wrapper of the ETH Zurich monitoring framework.
   */
  class CollectorMonitor: public Monitor<double>
  {


    public:

      /**
       * @brief define the type of the elements stored in the monitor
       */
      using value_type = double;

      /**
      * @brief Wrapper to communicate with the ETH Zurich monitoring framework
      */
      class collector_interface
      {


        public:


          /**
           * @brief Start the target measure
           */
          virtual void start( void ) = 0;


          /**
           * @brief Stop the target measure
           */
          virtual void stop( void ) = 0;


          /**
           * @brief Retrives the value of the measure
           *
           * @return The numeric value of the measure
           */
          virtual value_type get( void ) = 0;

          /**
           * @brief Virtual destructor for the interface
           */
          virtual ~collector_interface(void) = default;
      };




      /****************************************************
       * COLLECTOR MONITOR METHODS
       ****************************************************/

      /**
       * @brief  Trivial constructor
       *
       * @param window_size The dimension of the observation window
       *
       */
      CollectorMonitor( const std::size_t window_size = 1 );


      /**
       * @brief  Actual constructor of the monitor
       *
       * @param topic The topic of interest of the moinitor
       * @param address The address of the MQTT broker
       * @param port The number of the port used to communicate with the MQTT broker
       * @param window_size The dimension of the observation window
       *
       * @details
       * This constructor creates a separate thread that communicate with the MQTT
       * broker, to start and stop measures using the wrapper interface
       *
       */
      CollectorMonitor(const std::string topic, const std::string address, const int port, const std::size_t window_size = 1);


      /**
       * @brief Start the measure
       */
      void start( void );


      /**
       * @brief Stop the measure
       */
      void stop( void );


    private:


      /**
       * @brief The data structure to handle the comminication with the ETHz monitoring framework
       */
      std::shared_ptr<collector_interface> interface;


      /**
       * @brief States if the measure is started
       */
      bool started;

  };

}

#endif // MARGOT_COLLECTOR_MONITOR_HDR
