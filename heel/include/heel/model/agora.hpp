#ifndef HEEL_MODEL_AGORA_HDR
#define HEEL_MODEL_AGORA_HDR

#include <string>
#include <vector>

#include <heel/model/knob.hpp>
#include <heel/model/metric.hpp>
#include <heel/model/parameter.hpp>

namespace margot {
namespace heel {

struct agora_model {
  bool enabled;
  std::string url;
  std::string username;
  std::string password;
  std::string qos;
  std::string broker_ca;
  std::string client_cert;
  std::string client_key;
  std::string doe_plugin;
  std::string clustering_plugin;
  std::vector<pair_property> doe_parameters;
  std::vector<pair_property> clustering_parameters;
};

// this function validates an agora model to enforce a correct configuration of the application
void validate(agora_model& model, const std::vector<metric_model>& metrics,
              const std::vector<knob_model>& knobs);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_AGORA_HDR