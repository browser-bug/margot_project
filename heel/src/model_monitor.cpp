#include <algorithm>
#include <map>
#include <stdexcept>
#include <vector>

#include <boost/algorithm/string.hpp>

#include <heel/logger.hpp>
#include <heel/model_monitor.hpp>
#include <heel/typer.hpp>

// helper function that generates a spec for a custom monitor
inline margot::heel::monitor_spec gen_custom_spec(const std::string& monitor_type) {
  // get the type of a monitor
  const std::string real_type = margot::heel::sanitize_type(monitor_type);
  if (real_type.compare("string") == 0) {
    margot::heel::error("Unable to generate a monitor of strings");
    throw std::runtime_error("custom monitor: unsupported type");
  }
  return {"margot::Monitor<" + real_type + ">",
          "margot/monitor.hpp",
          real_type,
          "",
          "push",
          {{margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
          {},
          {{margot::heel::parameter_types::VARIABLE, "element", monitor_type}}};
}

// these are the required information for each type monitor that we have implemented in margot
// NOTE: the actual value type of each monitor should be taken directly from its class... However, since
//       we can't avoid a manual update of this map, it makes no sense to bring mARGOt as dependency just
//       for this reason, it will raise too much the complexity of the building system
static std::map<std::string, margot::heel::monitor_spec> known_monitors = {
    {"collector",
     {"margot::CollectorMonitor",
      "margot/collector_monitor.hpp",
      margot::heel::typer<double>::get(),
      "start",
      "stop",
      {{margot::heel::parameter_types::VARIABLE, "topic", "std::string"},
       {margot::heel::parameter_types::VARIABLE, "address", "std::string"},
       {margot::heel::parameter_types::VARIABLE, "port", "int"},
       {margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
      {},
      {}}},
    {"energy",
     {"margot::EnergyMonitor",
      "margot/energy_monitor.hpp",
      margot::heel::typer<long double>::get(),
      "start",
      "stop",
      {{margot::heel::parameter_types::IMMEDIATE, "margot::EnergyMonitor::Domain::Cores",
        "margot::EnergyMonitor::Domain"},
       {margot::heel::parameter_types::IMMEDIATE, "{}", "std::vector<std::size_t>"},
       {margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
      {},
      {}}},
    {"frequency",
     {"margot::FrequencyMonitor",
      "margot/frequency_monitor.hpp",
      margot::heel::typer<unsigned int>::get(),
      "",
      "measure",
      {{margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
      {},
      {}}},
    {"memory",
     {"margot::MemoryMonitor",
      "margot/memory_monitor.hpp",
      margot::heel::typer<std::size_t>::get(),
      "",
      "extractMemoryUsage",
      {{margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
      {},
      {}}},
    {"odroid_energy",
     {"margot::OdroidEnergyMonitor",
      "margot/odroid_energy_monitor.hpp",
      margot::heel::typer<long double>::get(),
      "start",
      "stop",
      {{margot::heel::parameter_types::IMMEDIATE, "margot::TimeUnit::MICROSECONDS", "margot::TimeUnit"},
       {margot::heel::parameter_types::VARIABLE, "polling_time_ms", "uint64_t"},
       {margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
      {},
      {}}},
    {"odroid_power",
     {"margot::OdroidPowerMonitor",
      "margot/odroid_power_monitor.hpp",
      margot::heel::typer<float>::get(),
      "start",
      "stop",
      {{margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
      {},
      {}}},
    {"papi",
     {"margot::PapiMonitor",
      "margot/papi_monitor.hpp",
      margot::heel::typer<long long int>::get(),
      "start",
      "stop",
      {{margot::heel::parameter_types::IMMEDIATE, "margot::PapiEvent::L1_MISS", "margot::PapiEvent"},
       {margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
      {},
      {}}},
    {"process_cpu",
     {"margot::ProcessCpuMonitor",
      "margot/process_cpu_usage_monitor.hpp",
      margot::heel::typer<float>::get(),
      "start",
      "stop",
      {{margot::heel::parameter_types::IMMEDIATE, "margot::CounterType::SoftwareCounter",
        "margot::CounterType"},
       {margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
      {},
      {}}},
    {"system_cpu",
     {"margot::SystemCpuMonitor",
      "margot/system_cpu_usage_monitor.hpp",
      margot::heel::typer<float>::get(),
      "start",
      "stop",
      {{margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
      {},
      {}}},
    {"temperature",
     {"margot::TemperatureMonitor",
      "margot/temperature_monitor.hpp",
      margot::heel::typer<long long int>::get(),
      "",
      "measure",
      {{margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
      {},
      {}}},
    {"throughput",
     {"margot::ThroughputMonitor",
      "margot/throughput_monitor.hpp",
      margot::heel::typer<float>::get(),
      "start",
      "stop",
      {{margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
      {},
      {{margot::heel::parameter_types::VARIABLE, "amount_data", "float"}}}},
    {"time",
     {"margot::TimeMonitor",
      "margot/time_monitor.hpp",
      margot::heel::typer<unsigned long int>::get(),
      "start",
      "stop",
      {{margot::heel::parameter_types::IMMEDIATE, "margot::TimeUnit::MICROSECONDS", "margot::TimeUnit"},
       {margot::heel::parameter_types::IMMEDIATE, "1", "size_t"}},
      {},
      {}}}};

static const std::vector<std::string> available_statistics = {"average", "standard_deviation", "max", "min"};

void margot::heel::validate(monitor_model& model) {
  // check if the monitor has a type
  if (model.type.empty()) {
    margot::heel::error("The monitor \"", model.name, "\" must have a type");
    throw std::runtime_error("monitor model: monitor without a type");
  }

  // make sure that we support the given monitor type
  boost::trim(model.type);
  auto monitor_spec_it = known_monitors.find(model.type);
  if (monitor_spec_it == known_monitors.cend()) {
    // if reach this statement, is not a known monitor, but we could generate the spec on the fly if it is a
    // numeric type (i.e. custom monitor)
    try {
      const auto res = known_monitors.emplace(model.type, gen_custom_spec(model.type));
      monitor_spec_it = res.first;
    } catch (const std::runtime_error& e) {
      margot::heel::error("Unknown type \"", model.type, "\" for monitor \"", model.name, "\"");
      throw e;
    }
  }

  // now we need to consider all the parameters in the monitor and combine them with the ones in the
  // knowledge. The idea is that the user should either redefine the whole set, or use the default version
  // we define this lambda to apply this procedure to all the parameters. Moreover, we set as a prefix the
  // name of the monitor, to make the variable name unique
  const auto check_parameters = [&model](const std::vector<struct margot::heel::parameter>& default_params,
                                         std::vector<struct margot::heel::parameter>& params,
                                         const std::string& params_name) {
    if (params.empty()) {
      params = default_params;
    } else if (params.size() != default_params.size()) {
      margot::heel::error("Unable to partially define the ", params_name, " parameters of monitor \"",
                          model.name, "\". Found ", params.size(), " parameter(s), ", default_params.size(),
                          " expected");
      throw std::runtime_error("monitor model: mismatch on " + params_name + " parameters");
    }
    std::transform(params.begin(), params.end(), params.begin(), [&model](margot::heel::parameter param) {
      if (param.type == margot::heel::parameter_types::VARIABLE) {
        param.content = model.name + "_" + param.content;
      }
      return param;
    });
  };

  // now we can check all the parameters of the monitor
  check_parameters(monitor_spec_it->second.default_param_initialization, model.initialization_parameters,
                   "initialization");
  check_parameters(monitor_spec_it->second.default_param_start, model.start_parameters, "start");
  check_parameters(monitor_spec_it->second.default_param_stop, model.stop_parameters, "stop");

  // now we need to check if the requested statistic to log is ok
  std::for_each(model.requested_statistics.begin(), model.requested_statistics.end(),
                [&model](const std::string& statistic_name) {
                  if (!std::any_of(available_statistics.begin(), available_statistics.end(),
                                   [&statistic_name](const std::string& known_name) {
                                     return known_name.compare(statistic_name) == 0;
                                   })) {
                    margot::heel::error("Unable to compute the statistic \"", statistic_name,
                                        "\" in monitor \"", model.name, "\"");
                    throw std::runtime_error("Unknown log statistic");
                  }
                });
}
