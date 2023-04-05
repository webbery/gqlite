#pragma once
#include "base/type.h"
#include "Node.h"
#include "json.hpp"
#include <list>
#include <stdexcept>

class GEntityNode: public GNode {
public:
  template<typename T>
  T value(attr_t attribute) {
    // AttributeKind kind = attributeKind(attribute);
  }

  GEntityNode(node_t id, group_t gid): _id(id), _gid(gid) {
    _status.updated = false;
  }

  void setProperties(const nlohmann::json& properties) { _properties = properties; }

  group_t gid() const { return _gid; }
  const nlohmann::json& attributes() const { return _properties; }
  node_t id() const { return _id; }

  void add_edge(const edge2_t& edge) { _edgesID.push_back(edge); }

  bool isUpdate() { return _status.updated; }

  void setUpdate(bool update) { _status.updated = update; }

  edge2_t next(const edge2_t& edge) {
    for (auto itr = _edgesID.begin(); itr != _edgesID.end(); ++itr) {
      if (*itr == edge) {
        ++itr;
        if (itr == _edgesID.end()) return *_edgesID.begin();
        return *itr;
      }
    }
    throw std::runtime_error("next edge not find.");
  }

  edge2_t prev(const edge2_t& edge) {
    for (auto itr = _edgesID.rbegin(); itr != _edgesID.rend(); ++itr) {
      if (*itr == edge) {
        ++itr;
        if (itr == _edgesID.rend()) return *_edgesID.rbegin();
        return *itr;
      }
    }
    throw std::runtime_error("prev edge not find.");
  }

  const std::list<edge2_t>& edges() const { return _edgesID; }
  
private:
  node_t _id;
  group_t _gid;
  nlohmann::json _properties;

  edge2_t _eid;
  std::list<edge2_t> _edgesID;
};