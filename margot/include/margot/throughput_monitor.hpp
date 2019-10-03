/* core/throughput_monitor.hpp
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

#ifndef MARGOT_THROUGHPUT_MONITOR_HDR
#define MARGOT_THROUGHPUT_MONITOR_HDR

#include <chrono>

#include "margot/monitor.hpp"

namespace margot {

/**
 * @brief  The throughput monitor
 *
 * @details
 * This use the std::chrono interface to gather the execution time
 * using a steady clock, if available. Otherwise it uses the monotonic_clock
 *
 * @note
 * The throughput is measured as [data]/seconds; however the resolution is in
 * microseconds, thus the monitor functionality should last at least 1us
 */
class ThroughputMonitor : public Monitor<float> {
 public:
  /**
   * @brief define the throughput_monitor type
   */
  using value_type = float;

  /****************************************************
   * Throughput Monitor methods
   ****************************************************/

  /**
   * @brief  Default constructor
   *
   * @param window_size The maximum number of elements stored in a monitor
   *
   */
  ThroughputMonitor(const std::size_t window_size = 1);

  /**
   * @brief  Start the observation
   *
   */
  void start();

  /**
   * @brief  Stop the observation and push the new data in the buffer
   *
   * @param [in] data The number of data processed in the observed execution time
   *
   */
  void stop(float data);

 private:
  /**
   * @brief The starting time when the measure is started
   */
  std::chrono::steady_clock::time_point tStart;

  /**
   * @brief States if a measure is started
   */
  bool started;
};

}  // namespace margot

#endif  // MARGOT_THROUGHPUT_MONITOR_HDR
