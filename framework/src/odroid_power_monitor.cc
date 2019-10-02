/* core/odroid_power_monitor.cc
 * Copyright (C) 2017 Gianluca Palermo
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

#include <margot/odroid_power_monitor.hpp>

#include <unistd.h>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>

/*
 * 0040	A15-BIG
 * 0045 A7-LITTLE
 * 0041 MEM
 * 0044 GPU
 */

namespace margot {

inline void readBigPower(double* power) {
  std::shared_ptr<FILE> fp(fopen("/sys/bus/i2c/drivers/INA231/2-0040/sensor_W", "r"), fclose);
  int result = ::fscanf(fp.get(), "%lf", power);

  // check if it's correct
  if (result == EOF) {
    throw std::runtime_error(
        "[odroid_power_monitor_t] Error: unable to parse the /sys/bus/i2c/drivers/INA231/2-0040/sensor_W "
        "file (BIG): reached end of file.");
  }
}

inline void readLittlePower(double* power) {
  std::shared_ptr<FILE> fp(fopen("/sys/bus/i2c/drivers/INA231/2-0045/sensor_W", "r"), fclose);
  int result = ::fscanf(fp.get(), "%lf", power);

  // check if it's correct
  if (result == EOF) {
    throw std::runtime_error(
        "[odroid_power_monitor_t] Error: unable to parse the /sys/bus/i2c/drivers/INA231/2-0045/sensor_W "
        "file (LITTLE): reached end of file.");
  }
}

inline void readMemoryPower(double* power) {
  std::shared_ptr<FILE> fp(fopen("/sys/bus/i2c/drivers/INA231/2-0041/sensor_W", "r"), fclose);
  int result = ::fscanf(fp.get(), "%lf", power);

  // check if it's correct
  if (result == EOF) {
    throw std::runtime_error(
        "[odroid_power_monitor_t] Error: unable to parse the /sys/bus/i2c/drivers/INA231/2-0041/sensor_W "
        "file (MEM): reached end of file.");
  }
}

inline void readGpuPower(double* power) {
  std::shared_ptr<FILE> fp(fopen("/sys/bus/i2c/drivers/INA231/2-0044/sensor_W", "r"), fclose);
  int result = ::fscanf(fp.get(), "%lf", power);

  // check if it's correct
  if (result == EOF) {
    throw std::runtime_error(
        "[odroid_power_monitor_t] Error: unable to parse the /sys/bus/i2c/drivers/INA231/2-0044/sensor_W "
        "file (GPU): reached end of file.");
  }
}

inline void checkTotal(uint64_t total) {
  if (total == 0) {
    throw std::runtime_error("[odroid_power_monitor_t] Error: no power consumed (Something went wrong?)");
  }
}

OdroidPowerMonitor::OdroidPowerMonitor(const std::size_t window_size) : Monitor(window_size) {
  started = false;
}

void OdroidPowerMonitor::start() {
  if (started) {
    return;
  }

  started = true;

  // This is not an intervall monitor
}

void OdroidPowerMonitor::stop() {
  // get the new measure
  double total_power = 0, power_big = 0, power_little = 0, power_gpu = 0, power_memory = 0;

  if (!started) {
    return;
  }

  started = false;

  readBigPower(&power_big);
  readLittlePower(&power_little);
  readGpuPower(&power_gpu);
  readMemoryPower(&power_memory);
  total_power = power_big + power_little + power_gpu + power_memory;

  push(total_power);
}

}  // namespace margot
