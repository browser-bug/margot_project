/* core/energy_monitor.hpp
 * Copyright (C) 2017 Davide Gadioli, Emanuele Vitali
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

#ifndef MARGOT_ENERGY_MONITOR_HDR
#define MARGOT_ENERGY_MONITOR_HDR

#include <functional>
#include <vector>
#include <cstddef>

#include "margot/monitor.hpp"

namespace margot
{

  /**
   * @brief  The energy monitor
   *
   * @details
   * All the measures are expressed in uJ
   * For the measurement it uses the rapl framework, therefore it works only on
   * intel platforms that support that framework.
   */
  class EnergyMonitor: public Monitor<long double>
  {


    public:


      /**
       * @brief This enums represents the domain of interest for the monitor
       */
      enum class Domain : unsigned int
      {
        Cores,
        Uncores,
        Ram,
        Package
      };

      /**
       * @brief define the type of the elements stored in the monitor
       */
      using value_type = long double;




      /****************************************************
       * ENERGY MONITOR METHODS
       ****************************************************/


      /**
       * @brief Trivial constructor
       *
       * @param window_size The dimension of the observation window
       */
      EnergyMonitor( const std::size_t window_size = 1 );


      /**
       * @brief  Default constructor of the monitor
       *
       * @param [in] interested_domain The enumerator that indetifies the domain of interest
       * @param [in] target_packages A std::vector of int that identifies the packages of interest
       * @param [in] window_size The dimension of the observation window
       *
       * @details
       * The RAPL framework takes measures per package. It is possible to select which packages
       * are of interest for the monitor. By default it selects all the available packages.
       * If there are more than one package, the monitor summs the measurement
       */
      EnergyMonitor(const Domain interested_domain, const std::vector<std::size_t> target_packages = {}, const std::size_t window_size = 1);


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
       * @brief A reference to function used to extract the energy
       */
      std::function<std::vector <std::pair<unsigned long long int, unsigned long long int>> (void)> extractor;


      /**
       * @brief The previous value of the counter (used to compute the measure by difference)
       */
      std::vector <std::pair<unsigned long long int, unsigned long long int>> previous_measure;

      /**
       * @brief States if the measure is started
       */
      bool started;
  };

}

#endif // MARGOT_ENERGY_MONITOR_HDR
