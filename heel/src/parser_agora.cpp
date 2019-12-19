#include <string>

#include <heel/model_agora.hpp>
#include <heel/parser_agora.hpp>
#include <heel/parser_parameter.hpp>
#include <heel/parser_tags.hpp>
#include <heel/parser_utils.hpp>

// this function basically fetch all the properties inside an agora element, with special care for the doe
// properties, that can vary according to the selected doe plugin
void margot::heel::parse(agora_model& agora, const boost::property_tree::ptree& agora_node) {
  margot::heel::parse_element(agora.url, agora_node, margot::heel::tag::url());
  margot::heel::parse_element(agora.username, agora_node, margot::heel::tag::username());
  margot::heel::parse_element(agora.password, agora_node, margot::heel::tag::password());
  margot::heel::parse_element(agora.qos, agora_node, margot::heel::tag::qos());
  margot::heel::parse_element(agora.broker_ca, agora_node, margot::heel::tag::broker_ca());
  margot::heel::parse_element(agora.client_cert, agora_node, margot::heel::tag::client_cert());
  margot::heel::parse_element(agora.client_key, agora_node, margot::heel::tag::client_key());
  margot::heel::parse_element(agora.doe_plugin, agora_node, margot::heel::tag::doe_plugin());
  margot::heel::parse_element(agora.clustering_plugin, agora_node, margot::heel::tag::clustering_plugin());
  margot::heel::parse_element(agora.url, agora_node, margot::heel::tag::url());
  margot::heel::parse_element(agora.url, agora_node, margot::heel::tag::url());
  margot::heel::parse_list(agora.doe_parameters, agora_node, margot::heel::tag::doe_parameters());
  margot::heel::parse_list(agora.clustering_parameters, agora_node,
                           margot::heel::tag::clustering_parameters());
}
