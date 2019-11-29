#include <boost/property_tree/ptree.hpp>

#include <heel/composer_agora.hpp>
#include <heel/composer_parameter.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_agora.hpp>
#include <heel/parser_tags.hpp>

void margot::heel::compose(boost::property_tree::ptree& agora_node, const agora_model& agora) {
  // write the base information to operate with agora
  agora_node.put(margot::heel::tag::url(), agora.url);
  agora_node.put(margot::heel::tag::username(), agora.username);
  agora_node.put(margot::heel::tag::password(), agora.password);
  agora_node.put(margot::heel::tag::qos(), agora.qos);
  agora_node.put(margot::heel::tag::broker_ca(), agora.broker_ca);
  agora_node.put(margot::heel::tag::client_cert(), agora.client_cert);
  agora_node.put(margot::heel::tag::client_key(), agora.client_key);
  agora_node.put(margot::heel::tag::doe_plugin(), agora.doe_plugin);
  agora_node.put(margot::heel::tag::clustering_plugin(), agora.clustering_plugin);

  // write the optional parameter for the
  margot::heel::add_list(agora_node, agora.doe_parameters, margot::heel::tag::doe_parameters());
  margot::heel::add_list(agora_node, agora.clustering_parameters, margot::heel::tag::clustering_parameters());
}