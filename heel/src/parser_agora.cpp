#include <string>

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

#include <heel/model_agora.hpp>
#include <heel/parser_agora.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

namespace pt = boost::property_tree;

// this function basically fetch all the properties inside an agora element, with special care for the doe
// properties, that can vary according to the selected doe plugin
margot::heel::agora_model margot::heel::parse_agora(const pt::ptree& block_node) {
  // get a default agora model
  margot::heel::agora_model model;
  model.enabled = false;

  // check if the user requested the agora model
  const boost::optional<const boost::property_tree::ptree&> node =
      block_node.get_child_optional(margot::heel::tag::agora());
  if (node) {
    // populate the model accordingly
    model.enabled = true;
    model.url = margot::heel::get(margot::heel::tag::url(), *node);
    model.username = margot::heel::get(margot::heel::tag::username(), *node);
    model.password = margot::heel::get(margot::heel::tag::password(), *node);
    model.qos = margot::heel::get(margot::heel::tag::qos(), *node);
    model.broker_ca = margot::heel::get(margot::heel::tag::broker_ca(), *node);
    model.client_cert = margot::heel::get(margot::heel::tag::client_cert(), *node);
    model.client_key = margot::heel::get(margot::heel::tag::client_key(), *node);
    model.doe_plugin = margot::heel::get(margot::heel::tag::doe_plugin(), *node);
    model.clustering_plugin = margot::heel::get(margot::heel::tag::clustering_plugin(), *node);
    margot::heel::visit_optional(
        margot::heel::tag::doe_parameters(), *node, [&model](const pt::ptree::value_type& p) {
          model.doe_parameters.emplace_back(
              margot::heel::pair_property{p.first, p.second.get<std::string>("", "")});
        });
    margot::heel::visit_optional(
        margot::heel::tag::clustering_parameters(), *node, [&model](const pt::ptree::value_type& p) {
          model.clustering_parameters.emplace_back(
              margot::heel::pair_property{p.first, p.second.get<std::string>("", "")});
        });
  }
  return model;
}
