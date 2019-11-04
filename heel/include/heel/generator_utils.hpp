#ifndef HEEL_GENERATOR_UTILS_HDR
#define HEEL_GENERATOR_UTILS_HDR

#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace margot {
namespace heel {

// this utility function performs the equivalent of "''.join()" in python, but over an iterable of objects not
// only strings. The conversion from the object to a string is performed by the given functor.
// NOTE: we can't put the oneliner version, since we need to print the first element first
template <class iterator_type, class functor_type>
inline std::string join(const iterator_type& begin, const iterator_type& end, const std::string& separator,
                        const functor_type& functor) {
  std::string string_initial_element = begin == end ? std::string() : functor(*begin);
  return string_initial_element +
         std::accumulate(std::next(begin), end, std::string{},
                         [&separator, &functor](const std::string& partial_result,
                                                const typename iterator_type::value_type& new_element) {
                           return partial_result + separator + functor(new_element);
                         });
}

// this struct represents a piece of content in cpp source/header file, plus all the headers that it requires
struct cpp_source_content {
  std::ostringstream content;
  std::vector<std::string> required_headers;
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_UTILS_HDR