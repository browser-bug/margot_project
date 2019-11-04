#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <heel/configuration_file.hpp>
#include <heel/generator_cpp_application_geometry_hdr.hpp>
#include <heel/generator_source_file.hpp>
#include <heel/json_parser.hpp>
#include <heel/model_application.hpp>
#include <heel/model_validate.hpp>

int main(int argc, char* argv[]) {
  // define the program options to describe where the source files will be generated
  std::filesystem::path path_conf_file;
  std::filesystem::path path_workspace(".");
  po::options_description desc("Allowed options");
  // clang-format off
  desc.add_options()
     ("help,h", "prints this message")
     ("configuration_file,c", po::value<std::filesystem::path>(&path_conf_file)->required(),
      "mARGOt configuration file path")
     ("workspace,w", po::value<std::filesystem::path>(&path_workspace)->default_value(path_workspace),
      "output folder path")
     ("op_files,o", po::value<std::vector<std::filesystem::path>>(),
      "operating points file path")
  ;
  // clang-format on
  po::positional_options_description p;
  p.add("op_files", -1);

  // parse the program options
  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
  if (vm.count("help") > 0) {
    std::cout << "This application generates the high-level interface of the mARGOt" << std::endl;
    std::cout << "dynamic autotuning framework. Moreover, it prints in the standard" << std::endl;
    std::cout << "output the list of generated files." << std::endl << std::endl;
    std::cout << "Usage: " << argv[0] << " [options] [path_op_list]*" << std::endl;
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }
  po::notify(vm);
  const auto op_list_files = !vm["op_files"].empty() ? vm["op_files"].as<std::vector<std::filesystem::path>>()
                                                     : std::vector<std::filesystem::path>();

  // load, parse, and validate the margot model from the configuration file
  margot::heel::configuration_file c;
  c.load(path_conf_file);
  margot::heel::application_model model = margot::heel::parse_json(c);
  margot::heel::validate(model);

  // do the same for the Operating Point lists (if any)
  std::for_each(op_list_files.begin(), op_list_files.end(), [&model](const std::filesystem::path& p) {
    margot::heel::configuration_file c;
    c.load(p);
    margot::heel::parse_json(c, model);
  });

  // now it is time to produce some output...
  margot::heel::source_file_generator g("prova.hpp", margot::heel::application_geometry_hpp_content(model));
  g.write_header(path_conf_file);
  return EXIT_SUCCESS;
}