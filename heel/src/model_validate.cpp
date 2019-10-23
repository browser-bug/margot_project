#include <algorithm>
#include <iterator>
#include <string>

#include <heel/logger.hpp>
#include <heel/model/application.hpp>
#include <heel/model/block.hpp>
#include <heel/model/validate.hpp>

// utility function that sets the content of a string, if it is empty
inline void set_default(std::string& string, const std::string& default_value, const std::string& what) {
  if (string.empty()) {
    string = default_value;
    margot::heel::warning("Setting the ", what, " to \"", default_value, "\" since no ", what,
                          " was provided");
  }
}

// forward-declaration of the functions that validates the application
template <class model_type>
inline void enforce_uniqueness(std::vector<model_type>& container, const std::string& what);

void margot::heel::validate(application_model& model) {
  // check if we need to set the application name and version
  set_default(model.name, "foo", "application name");
  set_default(model.version, "1.0", "application version");

  // enforce the uniqueness of the block's names
  enforce_uniqueness(model.blocks, "block");

  // since each block of the application is indipendent, the validation happens at block level
  std::for_each(model.blocks.begin(), model.blocks.end(), [](margot::heel::block_model& block) {
    // to perform cross-cheks in a proper way, ne need to enforce name uniqueness of sub sections
    enforce_uniqueness(block.monitors, "monitor");
    enforce_uniqueness(block.knobs, "knob");
    enforce_uniqueness(block.features.fields, "feature");
    enforce_uniqueness(block.monitors, "monitor");
  });
}

template <class model_type>
inline void enforce_uniqueness(std::vector<model_type>& container, const std::string& what) {
  // moves all the elements with the same name at the end of the container
  const auto last_unique =
      std::unique(container.begin(), container.end(),
                  [](const model_type& a, const model_type& b) { return a.name.compare(b.name) == 0; });

  // if the last unique is not equal to end, it means that we have duplicates, print as warning the elements
  // that we are going to remove and let's do it
  if (last_unique != container.end()) {
    margot::heel::warning("Found ", std::distance(last_unique, container.end()), " ", what,
                          "(s) with non-unique names:");
    std::for_each(last_unique, container.end(), [&what](const model_type& model) {
      margot::heel::warning("\tRemoving duplicated ", what, " \"", model.name, "\"");
    });
    container.erase(last_unique, container.end());
  }
}