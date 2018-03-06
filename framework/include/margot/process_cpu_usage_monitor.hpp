/* core/process_cpu_usage.hpp
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

#ifndef MARGOT_PROCESS_CPU_USAGE_MONITOR_HDR
#define MARGOT_PROCESS_CPU_USAGE_MONITOR_HDR

#include <chrono>
#include <functional>
#include <cstdint>
#include <sys/time.h>

#include "margot/monitor.hpp"

namespace margot
{

  /**
   * @brief States the gather method
   *
   * @details
   * The Hardware counter is more precise, however if the process
   * is migrated then the measure is bogous
   */
  enum class CounterType : uint8_t
  {
    SoftwareCounter,
    HardwareCounter
  };


  /**
   * @brief  The Process CPU usage monitor
   *
   * @details
   * This class represent a monitor that observe the percentage of time that the application
   * has spent in user or system time over the observed period. This class use two methods
   * in order to collect the data. A Software Counter and an Hardware counter.
   * The former use the getrusage syscall, the latter use the  clock_gettime function.
   * The hardware counter retrieve a measure in nanoseconds, while the soft counter in milliseconds.
   * However if the process is migrated from one physical core to another, the value may be bogus.
   *
   * @note
   * The hardware counter is available only in Linux environment, so on Mac OS X this counter is not
   * available
   */
  class ProcessCpuMonitor: public Monitor<float>
  {


    public:


      /**
       * @brief define the type of the elements stored in the monitor
       */
      using value_type = float;


      /**
       * @brief  Constructor with a Software Counter
       *
       * @param window_size The dimension of the observation window
       *
       */
      ProcessCpuMonitor( const std::size_t window_size = 1 );


      /**
       * @brief  General constructor, to choose the counter type
       *
       * @param counter_type If the monitor use a Software or an Hardware counter
       *
       * @param window_size The dimension of the observation window
       *
       * @note
       * For execution time below 100ms is better to use the Hardware counter
       *
       */
      ProcessCpuMonitor( CounterType counter_type, const size_t window_size = 1 );



      /**
       * @brief  Start the observation
       *
       *  @note
       *  In an OpenCL context, the time spent building the kernel at Run-Time is counted.
       */
      void start();


      /**
       * @brief  Stop the observation and push the new data in the buffer
       */
      void stop();


    private:


      /**
       * @brief Wall-time time point
       */
      std::chrono::steady_clock::time_point tStart;


      /**
       * @brief The time point with hardware counter
       */
      timespec uStart;


      /**
       * @brief States if a measure is started
       */
      bool started;


      /**
       * @brief function pointer that computes the CPU time
       */
      std::function<void(timespec&)> getProcessTime;

  };

}

#endif // MARGOT_PROCESS_CPU_USAGE_MONITOR_HDR
