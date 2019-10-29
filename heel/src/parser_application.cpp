#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_application.hpp>
#include <heel/parser_application.hpp>
#include <heel/parser_block.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

// this struct is used to store the actual values of a tag
struct tag {
  inline static const std::string application(void) { return "application"; }
  inline static const std::string version(void) { return "version"; }
};

// this function parses the few information about the application, then it parse the main part of the
// configuration file: the blocks of the application
margot::heel::application_model margot::heel::parse_application(const pt::ptree& application_node) {
  return {margot::heel::get(tag::application(), application_node),
          margot::heel::get(tag::version(), application_node), margot::heel::parse_blocks(application_node)};
}
