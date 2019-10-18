#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <heel/configuration_file.hpp>
#include <heel/generator/description_verbose.hpp>
#include <heel/parser/knob.hpp>
#include <heel/parser/monitor.hpp>

int main(int argc, char* argv[]) {
  // load the configuration file
  margot::heel::configuration_file c;
  c.load(std::filesystem::path("../prova.json"));

  // get the root element of the configuration file
  const auto p = c.ptree();

  // parse them
  auto models{margot::heel::parse_knobs(p)};

  // print them
  for (const auto& model : models) {
    std::cout << description_verbose(model).str() << std::endl;
  }

  return EXIT_SUCCESS;
}