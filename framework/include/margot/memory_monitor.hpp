/* core/memory_monitor.hpp
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

#ifndef MARGOT_MEMORY_MONITOR_HDR
#define MARGOT_MEMORY_MONITOR_HDR

#include <cstddef>

#include "margot/monitor.hpp"

namespace margot
{

  /**
   * @brief  The Memory Monitor
   *
   * @template The type of the monitor
   *
   * @details
   * This class represent a monitor that observe the memory used by the application.
   * Moreover it provide a methods that get the used peak virtual memory.
   * It parses the /proc metafiles to read the measure, so this monitor is linux specific.
   * The unit of measure of the monitor is the kB.
   */
  class MemoryMonitor: public Monitor<std::size_t>
  {


    public:


      /**
       * @brief define the type of the elements stored in the monitor
       */
      using value_type = std::size_t;


      /**
       * @brief  The Memory Monitor default constructor
       *
       * @param [in] window_size The dimension of the observation window
       */
      MemoryMonitor( const std::size_t window_size = 1 );


      /**
       * @brief Read the memory usage of the application
       *
       * @details
       * The measure of the memory is extracted by parsing the /proc/self/statm
       * metafile. For this reason the measure doesn't require a start()/stop().
       * The measure is pushed inside the data buffer
       */
      void extractMemoryUsage();


      /**
       * @brief Get the virtual memory peak
       *
       * @return the value of the virtual memory peak
       *
       * @note
       * This value is not stored inside the data buffer, but is intended as an
       * utility value.
       */
      value_type extractVmPeakSize();

  };

}

#endif // MARGOT_MEMORY_MONITOR_HDR
