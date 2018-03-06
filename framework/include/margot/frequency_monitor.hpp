/* core/frequency_monitor.hpp
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

#ifndef MARGOT_FREQUENCY_MONITOR_HDR
#define MARGOT_FREQUENCY_MONITOR_HDR

#include "margot/monitor.hpp"

namespace margot
{

  /**
   * @brief  The frequency monitor
   *
   * @details
   * All the measures are expressed in Khz
   */
  class FrequencyMonitor: public Monitor<unsigned int>
  {


    public:


      /**
       * @brief define the type of the elements stored in the monitor
       */
      using value_type = unsigned int;


      /**
       * @brief Default constructor
       *
       * @param [in] window_size The dimension of the observation window
       *
       * @details
       * To acquire the frequency of a core, it parses the CPUfreq meta file,
       * for this reason, this monitor is Linux dependent
       */
      FrequencyMonitor( const std::size_t window_size = 1 );


      /**
       * @brief  Change the cores observed by the monitor
       *
       * @param [in] cores A std::vector with the id number of the cores of interest.
       *
       * @details
       * By default, the frequency monitor takes an average of the frequencies
       * of all the available cores. However, this method enable the user to
       * select only the cores of interest
       */
      void cores(std::vector< unsigned int > cores);


      /**
       * @brief Read the metafile and compute the measure
       */
      void measure(void);


    private:


      /**
       * @brief The set of cores of interest for the monitor
       */
      std::vector< unsigned int > interested_core;

  };

}

#endif // MARGOT_FREQUENCY_MONITOR_HDR
