#pragma once

#include "Context.h"
#include "plan/PlanNode.h"

class CreateSchemaNode : public GPlanNode {
public:
  CreateSchemaNode(GContext* ctx);
};