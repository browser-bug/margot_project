/* core/system_cpu_usage_monitor.hpp
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

#ifndef MARGOT_SYSTEM_CPU_USAGE_MONITOR_HDR
#define MARGOT_SYSTEM_CPU_USAGE_MONITOR_HDR

#include "margot/monitor.hpp"

namespace margot {

/**
 * @brief  The System CPU usage monitor
 *
 * @details
 * This class represent a monitor that observe the percentage of time that the whole system
 * has spent in user or system time over the observation period.
 * The measure is used parsing the /proc/stat metafile
 */
class SystemCpuMonitor : public Monitor<float> {
 public:
  /**
   * @brief define the type of the elements stored in the monitor
   */
  using value_type = float;

  /****************************************************
   * System CPU Usage Monitor methods
   ****************************************************/

  /**
   * @brief  Default constructor
   *
   * @param window_size The dimension of the observation window
   *
   */
  SystemCpuMonitor(const std::size_t window_size = 1);

  /**
   * @brief  Start the observation
   *
   */
  void start();

  /**
   * @brief  Stop the observation and push the new data in the buffer
   *
   */
  void stop();

 private:
  /**
   * @brief The total busy time of the system
   */
  uint64_t busy_time;

  /**
   * @brief The wall time elapsed
   */
  uint64_t total_time;

  /**
   * @brief States if a measure is started
   */
  bool started;
};

}  // namespace margot

#endif  // MARGOT_SYSTEM_CPU_USAGE_MONITOR_HDR
