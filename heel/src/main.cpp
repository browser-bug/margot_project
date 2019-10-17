#include <filesystem>
#include <iostream>
#include <string>

#include <heel/configuration_file.hpp>

int main(int argc, char *argv[]) {
  margot::heel::configuration_file c;
  c.load(std::filesystem::path("prova.json"));

  std::cout << c.to_string() << std::endl;

  return EXIT_SUCCESS;
}