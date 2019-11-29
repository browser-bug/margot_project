#ifndef HEEL_COMPOSER_UTILS_HDR
#define HEEL_COMPOSER_UTILS_HDR

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include <boost/property_tree/ptree.hpp>

namespace margot {
namespace heel {

// this is an utility function that add a string to a node without any key (to compose an array of values)
inline void compose(boost::property_tree::ptree& n, const std::string& s) { n.put("", s); }

// this is an utility function that adds a vector of element's model to a ptree node. To avoid to specify a
// lambda that performs the composition of the element T, we rely on the developer to make sure that the
// function margot::heel::compose of T is in scope, when this template is instantiated
template <class T>
void add_list(boost::property_tree::ptree& parent_node, const std::vector<T>& elements,
              const std::string& tag_name) {
  if (!elements.empty()) {
    boost::property_tree::ptree elements_node;
    std::for_each(elements.begin(), elements.end(), [&elements_node](const T& element) {
      boost::property_tree::ptree element_node;
      margot::heel::compose(element_node, element);
      elements_node.push_back(std::make_pair("", element_node));
    });
    parent_node.add_child(tag_name, elements_node);
  }
}

// this is an utility function that compose and adds the specified element in the parent node. To avoid to
// specify a lambda that performs the composition of the element T, we rely on the developer to make sure that
// the function margot::heel::compose of T is in scope, when this template is instantiated
template <class T>
inline void add_element(boost::property_tree::ptree& parent_node, const T& element,
                        const std::string& tag_name) {
  if (!element.empty()) {
    boost::property_tree::ptree element_node;
    margot::heel::compose(element_node, element);
    parent_node.add_child(tag_name, element_node);
  }
}

}  // namespace heel
}  // namespace margot

#endif  // HELL_COMPOSER_UTILS_HDR