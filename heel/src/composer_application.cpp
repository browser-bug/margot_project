#include <boost/property_tree/ptree.hpp>

#include <heel/composer_application.hpp>
#include <heel/composer_block.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_application.hpp>
#include <heel/parser_tags.hpp>

void margot::heel::compose(boost::property_tree::ptree& root_node, const application_model& app) {
  // insert the basic information about the application
  root_node.put(margot::heel::tag::name(), app.name);
  root_node.put(margot::heel::tag::version(), app.name);

  // add the list of blocks
  margot::heel::add_list(root_node, app.blocks, margot::heel::tag::blocks());
}