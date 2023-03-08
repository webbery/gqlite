#pragma once
#include "plan/PlanNode.h"

class GUpsetVertex : public GPlanNode {
public:
  GUpsetVertex() {
    _kind = GPlanKind::UpsetVertex;
  }
};

class GUpsetEdge : public GPlanNode {
public:
  GUpsetEdge() {
    _kind = GPlanKind::UpsetEdge;
  }
};

class GDeleteVertex : public GPlanNode {
public:
  GDeleteVertex() {
    _kind = GPlanKind::DeleteVertex;
  }
};

class GDeleteEdge : public GPlanNode {
public:
  GDeleteEdge() {
    _kind = GPlanKind::DeleteEdge;
  }
};

