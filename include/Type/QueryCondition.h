#pragma once
#include "base/Clause.h"
#include "base/list.h"
#include "Type/Type.h"

class GQueryConditionClause: public GClause {
public:
  virtual int Parse(struct gast* ast);

  GConditions _preds;
};