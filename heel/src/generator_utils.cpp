#include <algorithm>
#include <sstream>
#include <string>

#include <heel/generator_utils.hpp>

namespace margot {
namespace heel {

void append(cpp_source_content& destination, const cpp_source_content& source, const std::string& prefix) {
  std::copy(source.required_headers.begin(), source.required_headers.end(),
            std::back_inserter(destination.required_headers));
  std::istringstream content(source.content.str());
  for (std::string line; std::getline(content, line); /* already handled */) {
    destination.content << prefix << line << std::endl;
  }
}

}  // namespace heel
}  // namespace margot