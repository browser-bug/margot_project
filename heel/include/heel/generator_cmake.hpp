#ifndef HEEL_GENERATOR_CMAKE_HDR
#define HEEL_GENERATOR_CMAKE_HDR

#include <filesystem>
#include <vector>

#include <heel/generator_source_file.hpp>

namespace margot {
namespace heel {

void generate_cmakelists(const std::filesystem::path& cmake_file_path,
                         const std::filesystem::path& header_path,
                         const std::vector<source_file_generator>& headers,
                         const std::vector<source_file_generator>& sources);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_CMAKE_HDR