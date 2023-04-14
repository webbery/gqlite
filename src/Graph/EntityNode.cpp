#include "Graph/EntityNode.h"

void GEntityNode::setProperty(const std::string& key, const attribute_t& value) {
  _attributes.insert({ key, value });
}

std::list<attribute_t> GEntityNode::attribute(const std::string& key) {
  std::list<attribute_t> ret;
  auto itr = _attributes.lower_bound(key);
  auto end = _attributes.upper_bound(key);
  for (; itr != end; ++itr) {
    ret.push_back(itr->second);
  }
  return ret;
}