#ifndef HEEL_CONFIGURATION_FILE_HDR
#define HEEL_CONFIGURATION_FILE_HDR

#include <filesystem>
#include <string>

#include <boost/property_tree/ptree.hpp>

namespace margot {
namespace heel {

class configuration_file {
  boost::property_tree::ptree content;

 public:
  // I/O functions from file
  void load_json(const std::filesystem::path &file_path);
  void store_json(const std::filesystem::path &where) const;

  // I/O functions from std::string
  void load_json(const std::string &description);
  std::string to_json_string(void) const;

  // functions to retrieve the internal representation of the configuration file
  inline boost::property_tree::ptree &ptree(void) { return content; }
  inline const boost::property_tree::ptree &ptree(void) const { return content; }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_CONFIGURATION_FILE_HDR