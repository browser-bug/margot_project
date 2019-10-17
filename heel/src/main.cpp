#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <heel/configuration_file.hpp>
#include <heel/model/monitor.hpp>
#include <heel/parser/monitor.hpp>

int main(int argc, char *argv[]) {
  // load the configuration file
  margot::heel::configuration_file c;
  c.load(std::filesystem::path("prova.json"));

  // get the root element of the configuration file
  const auto p = c.ptree();

  // parse them
  auto monitor_models{margot::heel::parse_monitors(p)};

  return EXIT_SUCCESS;
}