/* core/monitor/odroid_energy_monitor.hpp
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


#include <margot/odroid_energy_monitor.hpp>

#include <cstddef>
#include <string>
#include <string>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <memory>
#include <thread>
#include <chrono>

/*
 * 0040	A15-BIG
 * 0045 A7-LITTLE
 * 0041 MEM
 * 0044 GPU
*/

namespace margot
{

  inline void readBigPower(long double* power)
  {
    std::shared_ptr<FILE> fp(fopen("/sys/bus/i2c/drivers/INA231/2-0040/sensor_W", "r"), fclose);

    if (fp == NULL)
    {
      throw std::runtime_error("[OdroidEnergyMonitor] Error: unable to open the /sys/bus/i2c/drivers/INA231/2-0040/sensor_W file (BIG)");
    }

    int result = ::fscanf(fp.get(), "%Lf", power);

    // check if it's correct
    if (result == EOF)
    {
      throw std::runtime_error("[OdroidEnergyMonitor] Error: unable to parse the /sys/bus/i2c/drivers/INA231/2-0040/sensor_W file (BIG): reached end of file.");
    }
  }

  inline void readLittlePower(long double* power)
  {
    std::shared_ptr<FILE> fp(fopen("/sys/bus/i2c/drivers/INA231/2-0045/sensor_W", "r"), fclose);

    if (fp == NULL)
    {
      throw std::runtime_error("[OdroidEnergyMonitor] Error: unable to open the /sys/bus/i2c/drivers/INA231/2-0045/sensor_W file (LITTLE)");
    }

    int result = ::fscanf(fp.get(), "%Lf", power);

    // check if it's correct
    if (result == EOF)
    {
      throw std::runtime_error("[OdroidEnergyMonitor] Error: unable to parse the /sys/bus/i2c/drivers/INA231/2-0045/sensor_W file (LITTLE): reached end of file.");
    }
  }

  inline void readMemoryPower(long double* power)
  {
    std::shared_ptr<FILE> fp(fopen("/sys/bus/i2c/drivers/INA231/2-0041/sensor_W", "r"), fclose);

    if (fp == NULL)
    {
      throw std::runtime_error("[OdroidEnergyMonitor] Error: unable to open the /sys/bus/i2c/drivers/INA231/2-0041/sensor_W file (MEM)");
    }

    int result = ::fscanf(fp.get(), "%Lf", power);

    // check if it's correct
    if (result == EOF)
    {
      throw std::runtime_error("[OdroidEnergyMonitor] Error: unable to parse the /sys/bus/i2c/drivers/INA231/2-0041/sensor_W file (MEM): reached end of file.");
    }
  }

  inline void readGpuPower(long double* power)
  {
    std::shared_ptr<FILE> fp(fopen("/sys/bus/i2c/drivers/INA231/2-0044/sensor_W", "r"), fclose);

    if (fp == NULL)
    {
      throw std::runtime_error("[OdroidEnergyMonitor] Error: unable to open the /sys/bus/i2c/drivers/INA231/2-0044/sensor_W file (GPU)");
    }

    int result = ::fscanf(fp.get(), "%Lf", power);

    // check if it's correct
    if (result == EOF)
    {
      throw std::runtime_error("[OdroidEnergyMonitor] Error: unable to parse the /sys/bus/i2c/drivers/INA231/2-0044/sensor_W file (GPU): reached end of file.");
    }
  }

  inline void checkTotal(uint64_t total)
  {

    if (total == 0)
    {
      throw std::runtime_error("[OdroidEnergyMonitor] Error: no power consumed (Something went wrong?)");
    }
  }

  long double readTotalPower()
  {
    long double total_power = 0, power_big = 0, power_little = 0, power_gpu = 0, power_memory = 0;
    readBigPower(&power_big);
    readLittlePower(&power_little);
    readGpuPower(&power_gpu);
    readMemoryPower(&power_memory);
    total_power = power_big + power_little + power_gpu + power_memory;
    return total_power;
  }

  void synchronous_power_call(const uint64_t polling_time_ms, bool& started, bool& end_monitor, long double& total_energy)
  {
    while (end_monitor == false)
    {
      if (started == true)
      {
        total_energy += (readTotalPower() * polling_time_ms); ///1000.0; (millijoule)
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(polling_time_ms));
    }
  }

  OdroidEnergyMonitor::OdroidEnergyMonitor(const std::size_t window_size): Monitor(window_size) {}

  OdroidEnergyMonitor::OdroidEnergyMonitor(TimeUnit time_measure, const uint64_t polling_time_ms, const std::size_t window_size): Monitor( window_size )
  {
    started = false;
    synchronous_thread_if_p = std::shared_ptr<synchronous_thread_if>(new synchronous_thread_if(polling_time_ms));
  }

  void OdroidEnergyMonitor::start()
  {
    if (started)
    {
      return;
    }

    started = true;
    synchronous_thread_if_p->start();
  }

  void OdroidEnergyMonitor::stop()
  {
    if (!started)
    {
      return;
    }

    started = false;
    push(synchronous_thread_if_p->stop());
  }

}
