/* core/temperature_monitor.hpp
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
#include <cstring>
#include <stdexcept>
#include <vector>

#include <margot/temperature_monitor.hpp>

static bool initialized = false;

int n_core_sensors() {
  // initialize the sensors
  if (!initialized) {
    sensors_init(NULL);
    initialized = true;
  }

  sensors_chip_name const* cn;
  int i = 0, c = 0;

  for (cn = sensors_get_detected_chips(NULL, &c); cn != NULL; cn = sensors_get_detected_chips(NULL, &c))
    if (strncmp(cn->prefix, "coretemp", 8) == 0) {
      sensors_feature const* feat;
      int f = 0;

      for (feat = sensors_get_features(cn, &f); feat != NULL; feat = sensors_get_features(cn, &f))
        if (feat->type == SENSORS_FEATURE_TEMP) {
          i++;
        }
    }

  return i;
}

namespace margot {

TemperatureMonitor::TemperatureMonitor(const std::size_t window_size) : Monitor(window_size) {}

TemperatureMonitor::value_type TemperatureMonitor::TemperatureSensor::measure(void) {
  double avg = 0, temperature;

  // take the temperature for every sensor
  for (auto& sensor : sensors) {
    int result = sensors_get_value(sensor.cn, sensor.temp_input, &temperature);

#ifndef NDEBUG

    if (result < 0) {
      throw std::runtime_error("[TemperatureMonitor] Error: Unable to retrieve monitor informations");
    }

#endif

    avg += temperature;
  }

  if (ns > 0) {
    return static_cast<TemperatureMonitor::value_type>(avg / ns);
  }

  return static_cast<TemperatureMonitor::value_type>(0);
}

TemperatureMonitor::TemperatureSensor::~TemperatureSensor(void) {
  sensors_cleanup();
  sensors.clear();
}

TemperatureMonitor::TemperatureSensor::TemperatureSensor(void)
    : ns(n_core_sensors()), nc(sysconf(_SC_NPROCESSORS_ONLN)) {
  // initialize the sensors
  if (!initialized) {
    sensors_init(NULL);
    initialized = true;
  }

  // resize the array of the sensors
  sensors.reserve(ns);
  // intialize the data structure
  sensors_chip_name const* cn;
  int c = 0;

  for (cn = sensors_get_detected_chips(NULL, &c); cn != NULL; cn = sensors_get_detected_chips(NULL, &c)) {
    // check if the sensor chip is about the cpu
    if (strcmp(cn->prefix, "coretemp") == 0) {
      // compute the number of sensor per cip
      const unsigned int num_sensor_per_cpu = ns / nc;
      // populate the sensor structure
      sensors_feature const* feat;
      int f = 0;

      for (feat = sensors_get_features(cn, &f); feat != NULL; feat = sensors_get_features(cn, &f))
        if (feat->type == SENSORS_FEATURE_TEMP) {
          // get the critical temperature of the sensor
          double critical_temp;
          sensors_get_value(cn, sensors_get_subfeature(cn, feat, SENSORS_SUBFEATURE_TEMP_CRIT)->number,
                            &critical_temp);
          // actually add the sensor
          sensors.push_back({feat->number, cn,
                             sensors_get_subfeature(cn, feat, SENSORS_SUBFEATURE_TEMP_INPUT)->number,
                             critical_temp, num_sensor_per_cpu});
        }
    }
  }
}

}  // namespace margot
