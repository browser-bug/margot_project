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

// this is an utility function that retrieves the value of a node as a string, if it exists. If the node does
// not exist, it returns an empty string
inline std::string get( const std::string& tag_name, const boost::property_tree::ptree& node) {
	const boost::optional<const boost::property_tree::ptree&> child = node.get_child_optional(tag_name);
	if (child) {
		return child->get<std::string>("","");
	} else {
		return "";
	}
}

// this is an utility function that generates a list of values (as strings), starting from a range node
std::vector<std::string> compute_range(const boost::property_tree::ptree& range_node,
                                       const std::string& value_type);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_UTILS_HDR