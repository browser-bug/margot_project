#include <cstddef>
#include <map>
#include <stdexcept>
#include <string>

#include <heel/model/monitor.hpp>
#include <heel/typer.hpp>

// these are the required information for each type monitor that we have implemented in margot
// NOTE: the actual value type of each monitor should be taken directly from its class... However, since
//       we can't avoid a manual update of this map, it makes no sense to bring mARGOt as dependency just
//       for this reason, it will raise too much the complexity of the building system
static const std::map<std::string, margot::heel::monitor_spec> known_monitors = {
    {"collector",
     {"margot::CollectorMonitor", "margot/collector_monitor.hpp", margot::heel::typer<double>::get(), "start",
      "stop"}},
    {"energy",
     {"margot::EnergyMonitor", "margot/energy_monitor.hpp", margot::heel::typer<long double>::get(), "start",
      "stop"}},
    {"frequency",
     {"margot::FrequencyMonitor", "margot/frequency_monitor.hpp", margot::heel::typer<unsigned int>::get(),
      "", "measure"}},
    {"memory",
     {"margot::MemoryMonitor", "margot/memory_monitor.hpp", margot::heel::typer<std::size_t>::get(), "",
      "extractMemoryUsage"}},
    {"odroid_energy",
     {"margot::OdroidEnergyMonitor", "margot/odroid_energy_monitor.hpp",
      margot::heel::typer<long double>::get(), "start", "stop"}},
    {"odroid_power",
     {"margot::OdroidPowerMonitor", "margot/odroid_power_monitor.hpp", margot::heel::typer<float>::get(),
      "start", "stop"}},
    {"papi",
     {"margot::PapiMonitor", "margot/papi_monitor.hpp", margot::heel::typer<long long int>::get(), "start",
      "stop"}},
    {"process_cpu",
     {"margot::ProcessCpuMonitor", "margot/process_cpu_usage_monitor.hpp", margot::heel::typer<float>::get(),
      "start", "stop"}},
    {"system_cpu",
     {"margot::SystemCpuMonitor", "margot/system_cpu_usage_monitor.hpp", margot::heel::typer<float>::get(),
      "start", "stop"}},
    {"temperature",
     {"margot::TemperatureMonitor", "margot/temperature_monitor.hpp",
      margot::heel::typer<long long int>::get(), "", "measure"}},
    {"throughput",
     {"margot::ThroughputMonitor", "margot/throughput_monitor.hpp", margot::heel::typer<float>::get(),
      "start", "stop"}},
    {"time",
     {"margot::TimeMonitor", "margot/time_monitor.hpp", margot::heel::typer<unsigned long int>::get(),
      "start", "stop"}},
};

// this is a special name for holding the type name of a generic monitor
static const std::string custom_monitor_type("custom");

// this is a simple function that retrieves the information of the target monitor
margot::heel::monitor_spec get_known_monitor_spec(const std::string& monitor_type) {
  const auto found_element = known_monitors.find(monitor_type);
  if (found_element == known_monitors.cend()) {
    throw std::runtime_error(" monitor_model: unknown monitor type \"" + monitor_type + "\"");
  }
  return found_element->second;
}

// this function requires additional information to fill the monitor spec
margot::heel::monitor_spec get_generic_monitor_spec(const std::string& monitor_type,
                                                    const std::string& value_type) {
  return {"margot::Monitor<" + value_type + ">", "margot/monitor.hpp", value_type, "", "push"};
}

// this function initialize the monitor model with all the boring information about the monitor
// implementation.
margot::heel::monitor_model margot::heel::create_monitor(const std::string& monitor_type,
                                                         const std::string& value_type) {
  return {
      "",  // we don't know the monitor name
      monitor_type.compare(custom_monitor_type) != 0 ? get_known_monitor_spec(monitor_type)
                                                     : get_generic_monitor_spec(monitor_type, value_type),
      {},  // we don't know about the initialization parameters
      {},  // neither about the start parameters
      {}   // or the stop parameters
  };
}
