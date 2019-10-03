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

#include <unistd.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

#include "margot/frequency_monitor.hpp"

namespace margot {

FrequencyMonitor::FrequencyMonitor(const std::size_t window_size) : Monitor(window_size) {
  // get the number of the processors
  unsigned int number_cores = 0;

  // find the number of cores
  bool found_core = true;

  while (found_core) {
    FILE* ref = fopen(std::string("/sys/devices/system/cpu/cpu" + std::to_string(number_cores) +
                                  "/cpufreq/scaling_cur_freq")
                          .c_str(),
                      "r");

    if (ref == NULL) {
      found_core = false;
    } else {
      interested_core.emplace_back(number_cores);
      ++number_cores;
      fclose(ref);
    }
  }

  // check the number of cores
  assert(number_cores > 0 && "Error: unable to detect the frequency of any core");
}

void FrequencyMonitor::measure() {
  // init the measures
  value_type value = 0, avg = 0;

  // take the temperature for every sensor
  for (const auto& cpuid : interested_core) {
    // open the file
    FILE* ref =
        fopen(std::string("/sys/devices/system/cpu/cpu" + std::to_string(cpuid) + "/cpufreq/scaling_cur_freq")
                  .c_str(),
              "r");

    // check if the file is open
    if (ref != NULL) {
      // read the result
      int result = ::fscanf(ref, "%u\n", &value);

      // close the file
      fclose(ref);

      // check about the readings
      assert(result != EOF && "Error: the frequency monitor is unable to read the freqency of a core");

      // summ it
      avg += result != EOF ? value / interested_core.size() : static_cast<value_type>(0);
    }
  }

  // push the new value
  push(avg);
}

void FrequencyMonitor::cores(std::vector<unsigned int> cores) {
  // check if we are able to observe the selected cores
#ifndef NDEBUG

  // check if the new cores are valid
  bool valid = true;

  // try to parse the file
  for (const auto& cpuid : cores) {
    FILE* ref =
        fopen(std::string("/sys/devices/system/cpu/cpu" + std::to_string(cpuid) + "/cpufreq/scaling_cur_freq")
                  .c_str(),
              "r");

    if (ref == NULL) {
      valid = false;
      break;
    } else {
      fclose(ref);
    }
  }

  // check that every thing is ok
  assert(valid && "Error: the frequency monitor is not able to read at least one core of the selected ones");
#endif  // NDEBUG

  // update the set of interested cores
  interested_core = std::move(cores);
}

}  // namespace margot
