#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#include <heel/generator_cmake.hpp>
#include <heel/generator_source_file.hpp>
#include <heel/generator_utils.hpp>
#include <heel/logger.hpp>

void margot::heel::generate_cmakelists(const std::filesystem::path& cmake_file_path,
                                       const std::filesystem::path& header_path,
                                       const std::vector<source_file_generator>& headers,
                                       const std::vector<source_file_generator>& sources) {
  // open the output file
  std::ofstream f;
  f.open(cmake_file_path);
  if (!f.good()) {
    margot::heel::error("Unable to write the file ", cmake_file_path);
    throw std::runtime_error("cmake gen: unable to generate the cmake file");
  }

  // write the content of the cmake file
  f << "cmake_minimum_required( VERSION 3.9 )" << std::endl;
  f << "project( MARGOT_ADAPTIVE_LAYER VERSION 3.0 LANGUAGES C CXX)" << std::endl;
  f << "set(margot_core_DIR ${MARGOT_CMAKE_INSTALL_PATH})" << std::endl;
  f << "find_package(margot_core REQUIRED)" << std::endl;
  f << "set( LIBRARY_HEADERS " << std::endl;
  f << margot::heel::join(headers.begin(), headers.end(), " ",
                          [](const margot::heel::source_file_generator& s) {
                            return std::string("\"") + std::string(s.get_file_path()) + std::string("\"");
                          })
    << ")" << std::endl;
  f << "set( LIBRARY_SOURCES ";
  f << margot::heel::join(sources.begin(), sources.end(), " ",
                          [](const margot::heel::source_file_generator& s) {
                            return std::string("\"") + std::string(s.get_file_path()) + std::string("\"");
                          })
    << ")" << std::endl;
  f << "add_library(margot_heel_interface STATIC ${LIBRARY_HEADERS} ${LIBRARY_SOURCES})" << std::endl;
  f << "add_library(margot::heel_interface ALIAS margot_heel_interface)" << std::endl;
  f << "target_include_directories(margot_heel_interface PRIVATE " << header_path << ")" << std::endl;
  f << "set_target_properties(margot_heel_interface PROPERTIES PUBLIC_HEADER \"${LIBRARY_HEADERS}\")"
    << std::endl;
  f << "target_link_libraries(margot_heel_interface PUBLIC margot::margot_core)" << std::endl;
  f << "target_compile_features( margot_heel_interface PRIVATE cxx_std_11 )" << std::endl;
  f << "target_compile_options( margot_heel_interface PRIVATE \"-Wall\" )" << std::endl;
  f << "target_compile_options( margot_heel_interface PRIVATE $<$<CONFIG:DEBUG>: -g -O0 >)" << std::endl;
  f << "target_compile_options( margot_heel_interface PRIVATE $<$<CONFIG:RELWITHDEBINFO>: -march=native "
       "-mtune=native -g -O2 >)"
    << std::endl;
  f << "target_compile_options( margot_heel_interface PRIVATE $<$<CONFIG:RELEASE>: -march=native "
       "-mtune=native "
       "-O3 >)"
    << std::endl;

  // finally, close the file
  f.close();
}
