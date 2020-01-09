#include <boost/property_tree/ptree.hpp>

#include <heel/composer_application.hpp>
#include <heel/composer_block.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_application.hpp>
#include <heel/parser_tags.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& root_node, const application_model& app) {
  // insert the basic information about the application
  root_node.put(tag::name(), app.name);
  root_node.put(tag::version(), app.name);

  // add the list of blocks
  add_list(root_node, app.blocks, tag::blocks());
}

}  // namespace heel
}  // namespace margot