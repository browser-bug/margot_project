#ifndef MARGOT_AGORA_MODEL_DOE_HPP
#define MARGOT_AGORA_MODEL_DOE_HPP

#include <deque>
#include <map>
#include <unordered_map>
#include <string>

#include "agora/logger.hpp"

namespace agora {

// <key = knob_name, value = knob_value>
using configuration_t = std::unordered_map<std::string, std::string>;
typedef struct configuration_model
{
  configuration_model() {}
  configuration_model(const std::string &config_id, const configuration_t &config, int num_explorations)
      : configuration_id(config_id), configuration(config), number_of_explorations(num_explorations)
  {}

  bool empty() { return configuration.empty(); }

  std::string configuration_id;
  configuration_t configuration;
  int number_of_explorations;
} configuration_model;

// TODO: we could do something cleaner like
// using doe_row = std::pair<int, configuration_model>
// where the <int> represents the number of explorations made

struct doe_model
{
  doe_model() {}
  doe_model(const doe_model &other_doe) : required_explorations(other_doe.required_explorations) {}

  doe_model &operator=(const doe_model &ldoe)
  {
    required_explorations = ldoe.required_explorations;
    return *this;
  }

  // assuming no duplicates will be added
  void add_config(const std::string &config_id, const configuration_t &config, const int required_number_of_observations)
  {
    required_explorations.emplace_front(config_id, config, required_number_of_observations);
  }

  // this method returns the next configuration to explore
  bool get_next(configuration_model &ref)
  {
    // check if we have an empty map or if we reached the end of it
    if (!required_explorations.empty())
    {
      ref = required_explorations.front();
      required_explorations.pop_front();

      if (--ref.number_of_explorations > 0)
      {
        // if we didn't exhaust this configuration, put it back at the end
        required_explorations.push_back(ref);
      }

      return true;
    }

    return false;
  }

  void clear() { required_explorations.clear(); }

  // data
  std::deque<configuration_model> required_explorations;
};

} // namespace agora

#endif // MODEL_DOE_HPP
