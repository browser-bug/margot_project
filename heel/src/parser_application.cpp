#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include <heel/model_application.hpp>
#include <heel/parser_application.hpp>
#include <heel/parser_block.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

namespace margot {
namespace heel {

// this function parses the few information about the application, then it parse the main part of the
// configuration file: the blocks of the application
void parse(application_model& application, const boost::property_tree::ptree& application_node) {
  parse_element(application.name, application_node, tag::name());
  parse_element(application.version, application_node, tag::version());
  parse_list(application.blocks, application_node, tag::blocks());
}

}  // namespace heel
}  // namespace margot