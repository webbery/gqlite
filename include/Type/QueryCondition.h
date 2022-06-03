#pragma once
#include "Type/Type.h"

class GQueryConditionClause {
public:
  virtual int Parse(struct gast* ast);

  GConditions _preds;
};