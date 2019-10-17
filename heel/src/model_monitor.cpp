#include <cstddef>
#include <map>
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

// this function generates a spec for a custom monitor, given the monitor type
inline margot::heel::monitor_spec generate_spec(const std::string& monitor_type) {
  return {"margot::Monitor<" + monitor_type + ">", "margot/monitor.hpp", monitor_type, "", "push"};
}

// this is a simple function that retrieves the information of the target monitor by using information from
// known monitors, or by generating a spec for the custom type
margot::heel::monitor_spec get_monitor_spec(const std::string& monitor_type) {
  const auto found_element = known_monitors.find(monitor_type);
  return found_element != known_monitors.cend() ? found_element->second : generate_spec(monitor_type);
}

// this function initialize the monitor model with all the boring information about the monitor
// implementation.
margot::heel::monitor_model margot::heel::create_monitor(const std::string& monitor_type) {
  return {
      "",                              // we don't know the monitor name
      get_monitor_spec(monitor_type),  // yeah, it's basically the only thing that we know
      {},                              // we don't know about the initialization parameters
      {},                              // neither about the start parameters
      {}                               // or the stop parameters
  };
}
