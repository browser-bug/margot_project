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


#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <unistd.h>


#include <margot/memory_monitor.hpp>

namespace margot
{

  static const long page_size = sysconf(_SC_PAGE_SIZE) / 1024;

  MemoryMonitor::MemoryMonitor( const std::size_t window_size ): Monitor( window_size ) {}



  void MemoryMonitor::extractMemoryUsage()
  {
    // get the monitor value
    value_type memory_usage_kb;
    std::shared_ptr<FILE> fp(fopen("/proc/self/statm", "r"), fclose);
    int result = ::fscanf(fp.get(), "%*s %zu", &memory_usage_kb);

    // check if it's correct
    if (result == EOF)
    {
      throw std::runtime_error("Error, can't get the memory measure");
    }

    // convert the measure to kylobyte
    memory_usage_kb *= page_size;

    // push the value
    push(memory_usage_kb);
  }



  MemoryMonitor::value_type MemoryMonitor::extractVmPeakSize()
  {
    std::shared_ptr<FILE> fp(fopen("/proc/self/status", "r"), fclose);
    value_type vmPeak_Kb = 0;
    char buf[256];

    while (!feof(fp.get()))
    {
      if (fgets(buf, 256, fp.get()) == NULL)
      {
        throw std::runtime_error("Error, can't get the VmPeakSize");
      }

      if (::strncmp(buf, "VmPeak:", 7))
      {
        continue;
      }

      ::sscanf(buf, "%*s %lu", &vmPeak_Kb);
      return vmPeak_Kb ;
    }

    return vmPeak_Kb;
  }




}
