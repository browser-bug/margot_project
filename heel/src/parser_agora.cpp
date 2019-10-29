#include <string>

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

#include <heel/model/agora.hpp>
#include <heel/parser/agora.hpp>
#include <heel/parser/utils.hpp>

namespace pt = boost::property_tree;

// this struct is used to store the actual values of a tag
struct tag {
  inline static const std::string agora(void) { return "agora"; }
  inline static const std::string url(void) { return "borker_url"; }
  inline static const std::string username(void) { return "broker_username"; }
  inline static const std::string password(void) { return "broker_password"; }
  inline static const std::string qos(void) { return "broker_qos"; }
  inline static const std::string broker_ca(void) { return "broker_ca"; }
  inline static const std::string client_cert(void) { return "client_cert"; }
  inline static const std::string client_key(void) { return "client_key"; }
  inline static const std::string doe_plugin(void) { return "doe_plugin"; }
  inline static const std::string clustering_plugin(void) { return "clustering_plugin"; }
  inline static const std::string doe_parameters(void) { return "doe_parameters"; }
  inline static const std::string clustering_parameters(void) { return "clustering_parameters"; }
};

// this function basically fetch all the properties inside an agora element, with special care for the doe
// properties, that can vary according to the selected doe plugin
margot::heel::agora_model margot::heel::parse_agora(const pt::ptree& block_node) {
  // get a default agora model
  margot::heel::agora_model model;
  model.enabled = false;

  // check if the user requested the agora model
  const boost::optional<const boost::property_tree::ptree&> node =
      block_node.get_child_optional(tag::agora());
  if (node) {
    // populate the model accordingly
    model.enabled = true;
    model.url = margot::heel::get(tag::url(), *node);
    model.username = margot::heel::get(tag::username(), *node);
    model.password = margot::heel::get(tag::password(), *node);
    model.qos = margot::heel::get(tag::qos(), *node);
    model.broker_ca = margot::heel::get(tag::broker_ca(), *node);
    model.client_cert = margot::heel::get(tag::client_cert(), *node);
    model.client_key = margot::heel::get(tag::client_key(), *node);
    model.doe_plugin = margot::heel::get(tag::doe_plugin(), *node);
    model.clustering_plugin = margot::heel::get(tag::clustering_plugin(), *node);
    margot::heel::visit_optional(tag::doe_parameters(), *node, [&model](const pt::ptree::value_type& p) {
      model.doe_parameters.emplace_back(
          margot::heel::pair_property{p.first, p.second.get<std::string>("", "")});
    });
    margot::heel::visit_optional(
        tag::clustering_parameters(), *node, [&model](const pt::ptree::value_type& p) {
          model.clustering_parameters.emplace_back(
              margot::heel::pair_property{p.first, p.second.get<std::string>("", "")});
        });
  }
  return model;
}
