#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <heel/configuration_file.hpp>
#include <heel/generator/description_verbose.hpp>
#include <heel/parser/features.hpp>
#include <heel/parser/knob.hpp>
#include <heel/parser/metric.hpp>
#include <heel/parser/monitor.hpp>

int main(int argc, char* argv[]) {
  // load the configuration file
  margot::heel::configuration_file c;
  c.load(std::filesystem::path("../prova.json"));

  // get the root element of the configuration file
  const auto p = c.ptree();

  // print information
  for (const auto& model : margot::heel::parse_monitors(p)) {
    std::cout << margot::heel::description_verbose(model).str() << std::endl;
  }
  for (const auto& model : margot::heel::parse_knobs(p)) {
    std::cout << margot::heel::description_verbose(model).str() << std::endl;
  }
  for (const auto& model : margot::heel::parse_metrics(p)) {
    std::cout << margot::heel::description_verbose(model).str() << std::endl;
  }
  std::cout << margot::heel::description_verbose(margot::heel::parse_features(p)).str() << std::endl;

  return EXIT_SUCCESS;
}