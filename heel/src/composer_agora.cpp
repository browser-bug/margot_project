#include <boost/property_tree/ptree.hpp>

#include <heel/composer_agora.hpp>
#include <heel/composer_parameter.hpp>
#include <heel/composer_utils.hpp>
#include <heel/model_agora.hpp>
#include <heel/parser_tags.hpp>

namespace margot {
namespace heel {

void compose(boost::property_tree::ptree& agora_node, const agora_model& agora) {
  // write the base information to operate with agora
  agora_node.put(tag::url(), agora.url);
  agora_node.put(tag::username(), agora.username);
  agora_node.put(tag::password(), agora.password);
  agora_node.put(tag::qos(), agora.qos);
  agora_node.put(tag::broker_ca(), agora.broker_ca);
  agora_node.put(tag::client_cert(), agora.client_cert);
  agora_node.put(tag::client_key(), agora.client_key);
  agora_node.put(tag::doe_plugin(), agora.doe_plugin);
  agora_node.put(tag::clustering_plugin(), agora.clustering_plugin);

  // write the optional parameter for the
  add_list(agora_node, agora.doe_parameters, tag::doe_parameters());
  add_list(agora_node, agora.clustering_parameters, tag::clustering_parameters());
}

}  // namespace heel
}  // namespace margot
