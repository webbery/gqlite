#pragma once
#include "Type/Type.h"
#include "gqlite.h"

class GGraphInterface {
public:
  virtual int query(gqlite_node*& nodes, const GConditions& pred) = 0;
};