#ifndef MARGOT_AGORA_MODEL_DOE_HPP
#define MARGOT_AGORA_MODEL_DOE_HPP

#include <map>
#include <string>
#include <vector>

#include "agora/logger.hpp"

namespace agora {

using configuration_model = std::unordered_map<std::string, std::string>;
// TODO: we could do something cleaner like
// using doe_row = std::pair<int, configuration_model>
// where the <int> represents the number of explorations made

struct doe_model
{
  doe_model() {}
  doe_model(const doe_model &other_doe)
      : required_explorations(other_doe.required_explorations), it(required_explorations.end()),
        number_of_explorations(other_doe.number_of_explorations)
  {
  }

  doe_model &operator=(const doe_model &ldoe)
  {
    required_explorations = ldoe.required_explorations;
    number_of_explorations = ldoe.number_of_explorations;
    it = required_explorations.end();
    return *this;
  }

  bool add_config(const std::string &config_id, const configuration_model &config, const int required_number_of_observations)
  {
    bool assignment_took_place = !required_explorations.insert_or_assign(config_id, config).second ||
                                 !number_of_explorations.insert_or_assign(config_id, required_number_of_observations).second;
    it = required_explorations.begin();
    return assignment_took_place;
  }

  void update_config(const std::string &config_id)
  {
    // remove the configuration in case we exausted all the explorations
    if (--number_of_explorations.at(config_id) <= 0)
      remove_config(config_id);
  }

  void remove_config(const std::string &config_id)
  {
    required_explorations.erase(config_id);
    number_of_explorations.erase(config_id);
    it = required_explorations.begin();
  }

  // this method returns the next configuration to explore
  // NOTE: the caller MUST check the iterator returned (it != required_explorations.end()) before dereferencing it
  std::map<std::string, configuration_model>::iterator get_next()
  {
    // check if we have an empty map or if we reached the end of it
    if (required_explorations.empty() || std::next(it) == required_explorations.end())
    {
      it = required_explorations.begin();
      return it;
    }

    return ++it;
  }

  void clear() {
    required_explorations.clear();
    number_of_explorations.clear();
    it = required_explorations.begin();
  }

  // key is the configuration_id
  std::map<std::string, configuration_model> required_explorations;
  std::map<std::string, configuration_model>::iterator it;
  std::map<std::string, int> number_of_explorations;
};

} // namespace agora

#endif // MODEL_DOE_HPP
