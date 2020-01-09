#include <sstream>
#include <string>

#include <boost/property_tree/json_parser.hpp>

#include <heel/configuration_file.hpp>

namespace margot {
namespace heel {

void configuration_file::load_json(const std::filesystem::path &file_path) {
  boost::property_tree::read_json(static_cast<std::string>(file_path), content);
}

void configuration_file::load_json(const std::string &description) {
  std::stringstream content_stream(description);
  boost::property_tree::read_json(content_stream, content);
}

void configuration_file::store_json(const std::filesystem::path &where) const {
  boost::property_tree::write_json(static_cast<std::string>(where), content);
}

std::string configuration_file::to_json_string(void) const {
  std::stringstream content_stream;
  boost::property_tree::write_json(content_stream, content);
  return content_stream.str();
}

}  // namespace heel
}  // namespace margot