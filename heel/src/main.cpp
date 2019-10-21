#include <filesystem>
#include <iostream>
#include <string>

#include <heel/configuration_file.hpp>
#include <heel/json_parser.hpp>
#include <heel/model/application.hpp>
#include <heel/print_application_model.hpp>

int main(int argc, char* argv[]) {
  // load the configuration file
  margot::heel::configuration_file c;
  c.load(std::filesystem::path("../prova.json"));

  // parse the configuration file to generate the application model
  margot::heel::application_model model = margot::heel::parse_json(c);

  // print the information about the application model (on the standard output)
  margot::heel::print_application_model(model, std::cout);
  return EXIT_SUCCESS;
}