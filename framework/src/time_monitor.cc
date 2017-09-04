/* core/time_monitor.hpp
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



#include "margot/time_monitor.hpp"


#include <ratio>
#include <stdexcept>


using std::chrono::duration_cast;
using std::chrono::duration;


namespace margot
{


  TimeMonitor::value_type getElapsedTimeNanoseconds(std::chrono::steady_clock::time_point tStart,
      std::chrono::steady_clock::time_point tStop)
  {
    TimeMonitor::value_type elapsedTime = duration_cast<std::chrono::nanoseconds>(tStop - tStart).count();
    return elapsedTime;
  }

  TimeMonitor::value_type getElapsedTimeMicroseconds(std::chrono::steady_clock::time_point tStart,
      std::chrono::steady_clock::time_point tStop)
  {
    TimeMonitor::value_type elapsedTime = duration_cast<std::chrono::microseconds>(tStop - tStart).count();
    return elapsedTime;
  }

  TimeMonitor::value_type getElapsedTimeMilliseconds(std::chrono::steady_clock::time_point tStart,
      std::chrono::steady_clock::time_point tStop)
  {
    TimeMonitor::value_type elapsedTime = duration_cast<std::chrono::milliseconds>(tStop - tStart).count();
    return elapsedTime;
  }

  TimeMonitor::value_type getElapsedTimeSeconds(std::chrono::steady_clock::time_point tStart,
      std::chrono::steady_clock::time_point tStop)
  {
    TimeMonitor::value_type elapsedTime = duration_cast<std::chrono::seconds>(tStop - tStart).count();
    return elapsedTime;
  }


  TimeMonitor::TimeMonitor(TimeUnit time_measure,
                           const std::size_t window_size): Monitor(window_size)
  {

    switch (time_measure)
    {
      case TimeUnit::MICROSECONDS:
        time_extractor = getElapsedTimeMicroseconds;
        break;

      case TimeUnit::MILLISECONDS:
        time_extractor = getElapsedTimeMilliseconds;
        break;

      case TimeUnit::SECONDS:
        time_extractor = getElapsedTimeSeconds;
        break;

      case TimeUnit::NANOSECONDS:
        time_extractor = getElapsedTimeNanoseconds;
        break;

      default:
        throw std::logic_error("DEFENSIVE PROGRAMMING: Undefined TimeUnit in the time monitor");
    }

    started = false;
  }


  TimeMonitor::TimeMonitor(const size_t window_size): Monitor(window_size)
  {
    time_extractor = getElapsedTimeMilliseconds;
    started = false;
  }


  void TimeMonitor::start()
  {

    if (started)
    {
      return;
    }

    started = true;

    t_start = std::chrono::steady_clock::now();

  }



  void TimeMonitor::stop()
  {

    if (!started)
    {
      return;
    }

    TimeMonitor::value_type time_elapsed = time_extractor(t_start, std::chrono::steady_clock::now());


    Monitor::push(time_elapsed);

    started = false;

  }



}
