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
void margot::heel::parse(application_model& application,
                         const boost::property_tree::ptree& application_node) {
  margot::heel::parse_element(application.name, application_node, margot::heel::tag::name());
  margot::heel::parse_element(application.version, application_node, margot::heel::tag::version());
  margot::heel::parse_list(application.blocks, application_node, margot::heel::tag::blocks());
}
