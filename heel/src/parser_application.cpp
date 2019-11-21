#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_application.hpp>
#include <heel/parser_application.hpp>
#include <heel/parser_block.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

// this function parses the few information about the application, then it parse the main part of the
// configuration file: the blocks of the application
margot::heel::application_model margot::heel::parse_application(const pt::ptree& application_node) {
  return {margot::heel::get(margot::heel::tag::application(), application_node),
          margot::heel::get(margot::heel::tag::version(), application_node),
          margot::heel::parse_blocks(application_node)};
}
