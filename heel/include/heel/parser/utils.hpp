#ifndef HEEL_PARSER_UTILS_HDR
#define HEEL_PARSER_UTILS_HDR

#include <string>
#include <vector>

#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

namespace margot {
namespace heel {

// this is an utility function that executes the functor over all the childs of an element with name
// "tag_name" within the given node, if any. It is meant to parse optional nodes
template <class functor_type>
inline void visit_optional(const std::string& tag_name, const boost::property_tree::ptree& node,
                           const functor_type& functor) {
  const boost::optional<const boost::property_tree::ptree&> child = node.get_child_optional(tag_name);
  if (child) {
    for (const auto& node_pair : *child) {
      functor(node_pair);  // first: node name as string | second: the node content as ptree
    }
  }
}

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_UTILS_HDR