#ifndef HEEL_GENERATOR_SOURCE_FILE_HDR
#define HEEL_GENERATOR_SOURCE_FILE_HDR

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <heel/generator_utils.hpp>
#include <heel/logger.hpp>

namespace margot {
namespace heel {

class source_file_generator {
  std::ofstream file_handler;
  std::vector<std::string> required_headers;
  const std::filesystem::path file_path;
  const std::string source_content;

 public:
  template <class... component_type>
  source_file_generator(const std::filesystem::path file_path, const component_type&... components)
      : file_path(file_path), source_content(fill(components...)) {}

  ~source_file_generator(void) {}

  void write_header(const std::filesystem::path& configuration_filepath = "") {
    // compose the include guard unique definition
    std::string filename = file_path.stem();
    std::transform(filename.begin(), filename.end(), filename.begin(),
                   [](typename std::string::value_type c) { return std::toupper(c); });
    const std::string include_guard_def = "MARGOT_" + filename + "_HDR";

    // write the preamble of the file
    open();
    file_handler << "#ifndef " << include_guard_def << std::endl;
    file_handler << "#define " << include_guard_def << std::endl << std::endl;
    internal_write(configuration_filepath);
    file_handler << "#endif // " << include_guard_def << std::endl << std::endl;
    close();
  }

  inline void write_source(const std::filesystem::path& configuration_filepath = "") {
    open();
    internal_write(configuration_filepath);
    close();
  }

 private:
  void open(void) {
    file_handler.open(file_path);
    if (!file_handler.good()) {
      margot::heel::error("Unable to write the file ", file_path);
      throw std::runtime_error("source gen: unable to write on file");
    }
  }

  void close(void) {
    if (file_handler.is_open()) {
      file_handler.close();
    }
  }

  void internal_write(const std::filesystem::path& configuration_filepath) {
    // print the warning about auto-generated files
    file_handler << "// WARNING:" << std::endl;
    file_handler << "// This file is automatically generated. Any manual change can be overwritten."
                 << std::endl;
    file_handler << "// To change the high level interface, change the configuration file(s)" << std::endl;

    // print the information about the generation date
    auto now = std::time(nullptr);
    char formatted_time[20];
    if (std::strftime(formatted_time, sizeof(formatted_time), "%F %T", std::localtime(&now))) {
      file_handler << "// Generation date: " << formatted_time << std::endl;
    }

    // print the information about the original config file (if any)
    if (!configuration_filepath.empty()) {
      file_handler << "// Configuration file path: " << configuration_filepath << std::endl;
    }
    file_handler << std::endl;

    // now print all the headers required for this source file
    std::sort(required_headers.begin(), required_headers.end());
    required_headers.erase(std::unique(required_headers.begin(), required_headers.end()),
                           required_headers.end());
    std::for_each(required_headers.begin(), required_headers.end(), [this](const std::string& header) {
      file_handler << "#include <" << header << ">" << std::endl;
    });
    file_handler << std::endl;

    // now we can print the content of the file
    file_handler << source_content << std::endl;
  }

  template <class... component_type>
  inline std::string fill(const cpp_source_content& c, const component_type&... remainder) {
    required_headers.insert(required_headers.end(), c.required_headers.begin(), c.required_headers.end());
    return c.content.str() + fill(remainder...);
  }

  template <class... component_type>
  inline std::string fill(const std::string& s, const component_type&... remainder) {
    return s + fill(remainder...);
  }

  inline std::string fill(void) { return std::string(); }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_SOURCE_FILE_HDR