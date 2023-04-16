#pragma once
#include "Edge.h"
#include "Graph/EntityNode.h"
#include "base/type.h"
#include "json.hpp"
#include "gutil.h"
#include <string>

enum class EdgeChangedStatus {
  Latest,
  SrcChanged,
  DstChanged,
  TwoSideChaned,
  PropertyChanged,
  NoneChanged,
};

class GEntityEdge : public GEdge {
public:
  GEntityEdge(GEntityNode* from, GEntityNode* to, bool direction = false, group_t gid = 0)
    :_gid(gid), _from(from), _to(to) {
      gql::edge_id eid = gql::make_edge_id(direction, from?from->id():(uint64_t)0, to?to->id(): (uint64_t)0);
      _id = gql::to_string(eid);

      _status.updated = false;
      _status.direction = eid._direction;

      _changed = EdgeChangedStatus::Latest;

      gql::release_edge_id(eid);
    }

  edge2_t id() { return _id; }
  group_t gid() { return _gid; }

  void setUpdate(bool state) { _status.updated = state; }

  EdgeChangedStatus status() { return _changed; }

  GEntityNode* from() { return _from; }
  GEntityNode* to() { return _to; }

  void setFrom(GEntityNode* from) { _from = from; }
  void setTo(GEntityNode* to) { _to = to; }

  void setChangedStatus(EdgeChangedStatus status) {
    _changed = status;
  }

  void setProperties(const nlohmann::json& properties) {
    _properties = properties;
    _changed = EdgeChangedStatus::PropertyChanged;
  }

  bool direction() const { return _status.direction; }
  bool direction() { return _status.direction; }

private:
  edge2_t _id;  // edge_t
  group_t _gid; // group id

  nlohmann::json _properties;
  
  GEntityNode* _from;
  GEntityNode* _to;

  EdgeChangedStatus _changed;
};