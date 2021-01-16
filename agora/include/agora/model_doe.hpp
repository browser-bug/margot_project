#ifndef MARGOT_AGORA_MODEL_DOE_HPP
#define MARGOT_AGORA_MODEL_DOE_HPP

#include <string>
#include <map>
#include <vector>

#include "agora/logger.hpp"

namespace agora {

// TODO: this can become something more detailed inside heel
using configuration_model = std::unordered_map<std::string, std::string>;

struct doe_model
{
  // this method adds a configuration
  inline bool add_config(const std::string &config_id, const configuration_model &config, const int required_number_of_observations)
  {
    return !required_explorations.insert_or_assign(config_id, config).second ||
           !number_of_explorations.insert_or_assign(config_id, required_number_of_observations).second;
  }

  inline void update_config(const std::string &config_id)
  {
    number_of_explorations.at(config_id)--;

    // remove the configuration in case we exausted all the explorations
    if (number_of_explorations.at(config_id) <= 0)
      remove_config(config_id);
  }

  // this method removes a configuration
  inline void remove_config(const std::string &config_id)
  {
    required_explorations.erase(config_id);
    number_of_explorations.erase(config_id);
  }

  // this method returns the next configuration to explore
  inline const std::map<std::string, configuration_model>::iterator get_next()
  {
    // we may have an empty map or one with only a single configuration left
    if (required_explorations.empty() || next == required_explorations.end())
    {
      next = required_explorations.begin();
      return next;
    }

    next++;

    return next;
  }

  // unique_id generator
  std::map<std::string, configuration_model> required_explorations;
  std::map<std::string, configuration_model>::iterator next = required_explorations.end();
  std::map<std::string, int> number_of_explorations;
};

} // namespace agora

#endif // MODEL_DOE_HPP
