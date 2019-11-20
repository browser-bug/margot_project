#include <algorithm>
#include <cstdint>
#include <sstream>
#include <string>

#include <heel/generator_utils.hpp>

void margot::heel::append(margot::heel::cpp_source_content& destination,
                          const margot::heel::cpp_source_content& source, const std::string& prefix,
                          const std::size_t min_line_merge) {
  std::size_t counter = 0;
  std::istringstream content(source.content.str());

  // at first we need to see if we care about storing the buffer in the destination
  std::ostringstream temp_buf;
  std::string line;
  while (std::getline(content, line)) {
    temp_buf << prefix << line << std::endl;
    if (counter < min_line_merge) {
      ++counter;
    } else {
      break;
    }
  }

  // if we reach this point and the counter is equal to the minumum number of lines, it means that we are
  // interested on appending this stream
  if (counter == min_line_merge) {
    destination.content << temp_buf.str();
    while (std::getline(content, line)) {
      destination.content << prefix << line << std::endl;
    }
    std::copy(source.required_headers.begin(), source.required_headers.end(),
              std::back_inserter(destination.required_headers));
  }
}