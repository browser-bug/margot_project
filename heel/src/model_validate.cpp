#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include <heel/logger.hpp>
#include <heel/model_agora.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/model_features.hpp>
#include <heel/model_knob.hpp>
#include <heel/model_metric.hpp>
#include <heel/model_monitor.hpp>
#include <heel/model_state.hpp>
#include <heel/model_validate.hpp>
#include <heel/typer.hpp>

// utility functions used in the validation process
inline void set_name(std::string& string, const std::string& what, const std::size_t counter) {
  if (string.empty()) {
    string = what + "_" + std::to_string(counter + 1);
    const std::string element_name = [&what, &counter](void) {
      if (counter == 0) {
        return "1st " + what;
      } else if (counter == 1) {
        return "2nd " + what;
      } else if (counter == 2) {
        return "3rd " + what;
      } else {
        return std::to_string(counter + 1) + "th " + what;
      }
    }();
    margot::heel::warning("Setting the ", element_name, " name to \"", string, "\" since none was provided");
  }
}
template <class model_type>
inline void set_name(std::vector<model_type>& container, const std::string& what);
template <class model_type>
inline void check_uniqueness(std::vector<model_type>& container, const std::string& what);

// this is the main function that validates the whole application model
void margot::heel::validate(application_model& model) {
  // check if we need to set the application name and version
  if (model.name.empty()) {
    model.name = "foo";
    margot::heel::warning("Setting the application name to \"", model.name, "\" since none was provided");
  }
  if (model.version.empty()) {
    model.version = "1.0";
    margot::heel::warning("Setting the application version to \"", model.version,
                          "\" since none was provided");
  }

  // enforce the uniqueness of the block's names
  check_uniqueness(model.blocks, "block");

  // since each block of the application is indipendent, the validation happens at block level
  std::for_each(model.blocks.begin(), model.blocks.end(), [](margot::heel::block_model& block) {
    // to perform cross-cheks in a proper way, ne need to enforce name uniqueness of sub sections
    check_uniqueness(block.monitors, "monitor");
    check_uniqueness(block.knobs, "knob");
    check_uniqueness(block.features.fields, "feature");
    check_uniqueness(block.metrics, "metric");
    check_uniqueness(block.states, "state");

    // now we need to make sure that there is no metric with the same name as a knob
    std::for_each(block.knobs.begin(), block.knobs.end(), [&block](const knob_model& knob) {
      if (std::any_of(block.metrics.begin(), block.metrics.end(),
                      [&knob](const metric_model& metric) { return metric.name.compare(knob.name) == 0; })) {
        margot::heel::error("Ambiguous name \"", knob.name, "\" in block \"", block.name,
                            "\": metrics and knobs can't have the same name");
        throw std::runtime_error("validation error: ambiguous naming");
      }
    });

    // now we need to validate all the sub-modules in isolation, and running cross-checks
    std::for_each(block.monitors.begin(), block.monitors.end(),
                  [](monitor_model& monitor) { margot::heel::validate(monitor); });
    std::for_each(block.knobs.begin(), block.knobs.end(),
                  [](knob_model& knob) { margot::heel::validate(knob); });
    std::for_each(block.metrics.begin(), block.metrics.end(),
                  [&block](metric_model& metric) { margot::heel::validate(metric, block.monitors); });
    margot::heel::validate(block.features);
    margot::heel::validate(block.agora, block.metrics, block.knobs);
    std::for_each(block.states.begin(), block.states.end(), [&block](state_model& state) {
      margot::heel::validate(state, block.metrics, block.knobs);
    });

    // now, we have to do the difficult part: figure out which is the the most suitable type to hold all the
    // knobs and the one for the metrics.
    std::vector<std::string> knob_types, metric_types;
    std::for_each(block.knobs.begin(), block.knobs.end(), [&knob_types](const knob_model& knob) {
      if (knob.type.compare("string") != 0) {
        knob_types.emplace_back(knob.type);
      }
    });
    std::for_each(block.metrics.begin(), block.metrics.end(),
                  [&metric_types](const metric_model& metric) { metric_types.emplace_back(metric.type); });
    if (knob_types.empty()) {  // if we have only string knobs, we can use int as enum types
      knob_types.emplace_back("int");
    }
    std::sort(knob_types.begin(), knob_types.end(), [](const std::string& a, const std::string& b) {
      const auto result = margot::heel::type_sorter(a, b);
      if (!result) {
        margot::heel::error(
            "Unable to deal with knobs of type \"", a, "\" and \"", b,
            "\", select a type which belong to the same category, i.e. signed, unsigned, and floating point");
        throw std::runtime_error("model vlidation: mismatch between knobs type");
      }
      return *result;
    });
    std::sort(metric_types.begin(), metric_types.end(), [](const std::string& a, const std::string& b) {
      const auto result = margot::heel::type_sorter(a, b);
      if (!result) {
        margot::heel::error(
            "Unable to deal with metrics of type \"", a, "\" and \"", b,
            "\", select a type which belong to the same category, i.e. signed, unsigned, and floating point");
        throw std::runtime_error("model vlidation: mismatch between metrics type");
      }
      return *result;
    });
    block.knobs_segment_type = !knob_types.empty() ? knob_types.back() : std::string();
    block.metrics_segment_type = !metric_types.empty() ? metric_types.back() : std::string();

    // now we need to enforce the constistency of the Operating Point geometry: either we have knobs and
    // metrics, or we don't have any of them;
    if ((block.knobs.empty() && !block.metrics.empty()) || (!block.knobs.empty() && block.metrics.empty())) {
      margot::heel::error("The block \"", block.name, "\" is partially managed, knobs or metrics missing");
      throw std::runtime_error("model vlidation: block partially defined");
    }
  });
}

template <class model_type>
inline void check_uniqueness(std::vector<model_type>& container, const std::string& what) {
  // at first we set all the elements with a default unique name, according to what they are
  set_name(container, what);

  // moves all the elements with the same name at the end of the container
  const auto last_unique =
      std::unique(container.begin(), container.end(),
                  [](const model_type& a, const model_type& b) { return a.name.compare(b.name) == 0; });

  // if the last unique is not equal to end, it means that we have duplicates, print as warning the elements
  // that we are going to remove and let's do it
  if (last_unique != container.end()) {
    margot::heel::error("Found ", std::distance(last_unique, container.end()), " ", what,
                        "(s) with non-unique names:");
    std::for_each(last_unique, container.end(), [&what](const model_type& model) {
      margot::heel::warning("\tFound duplicated ", what, " \"", model.name, "\"");
    });
    throw std::runtime_error("validation error: " + what + "s names must be unique");
  }
}

template <class model_type>
inline void set_name(std::vector<model_type>& container, const std::string& what) {
  std::size_t counter = 0;
  std::for_each(container.begin(), container.end(),
                [&what, &counter](model_type& model) { set_name(model.name, what, counter++); });
}